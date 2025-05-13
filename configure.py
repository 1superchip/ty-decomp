#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = [
    "GTYE69",  # USA, 0
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-1"
config.compilers_tag = "20240706"
config.dtk_tag = "v1.3.0"
config.objdiff_tag = "v2.4.0"
config.sjiswrap_tag = "v1.2.0"
config.wibo_tag = "0.6.11"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
    f"--defsym VERSION_{config.version}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = 96

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    # "-W all",
    "-O4,p",
    "-inline auto",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-RTTI off",
    "-fp_contract on",
    "-str reuse",
    # "-multibyte",  # For Wii compilers, replace with `-enc SJIS`
    "-i include",
    f"-i build/{config.version}/include",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-gccinc",
    "-common off",
    "-char signed",
    "-inline deferred,auto",
]

# DOL flags
cflags_dol = [
    *cflags_base,
    "-RTTI on",
    "-fp_contract off",
    "-O4,p",
]

# REL flags
cflags_rel = [
    *cflags_base,
    "-sdata 0",
    "-sdata2 0",
]

config.linker_version = "GC/1.3.2"


# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.2.5n",
        "cflags": cflags_base,
        "progress_category": "sdk",
        "objects": objects,
    }

def Dol(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": config.linker_version,
        "cflags": cflags_dol,
        "progress_category": "game",
        "host": False,
        "objects": objects,
    }

# Helper function for REL script objects
def Rel(lib_name: str, objects: List[Object]) -> Dict[str, Any]:
    return {
        "lib": lib_name,
        "mw_version": "GC/1.3.2",
        "cflags": cflags_rel,
        "progress_category": "game",
        "objects": objects,
    }


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [
    Dol(
        "dol",
        [
            Object(NonMatching, "ty/source/camera/CameraOverride.cpp"),
            Object(NonMatching, "ty/source/camera/GameCamera.cpp"),
            Object(Matching, "ty/source/camera/GameCameraTools.cpp"),
            Object(NonMatching, "ty/source/critters/Bird.cpp"),
            Object(NonMatching, "ty/source/critters/Cattle.cpp"),
            Object(NonMatching, "ty/source/critters/CritterDefs.cpp"),
            Object(NonMatching, "ty/source/critters/Critterfield.cpp"),
            Object(NonMatching, "ty/source/critters/Critters.cpp"),
            Object(NonMatching, "ty/source/critters/CuttleFish.cpp"),
            Object(NonMatching, "ty/source/critters/Fish.cpp"),
            Object(NonMatching, "ty/source/critters/FishShoal.cpp"),
            Object(NonMatching, "ty/source/critters/Fly.cpp"),
            Object(NonMatching, "ty/source/critters/Frog.cpp"),
            Object(NonMatching, "ty/source/critters/Gecko.cpp"),
            Object(NonMatching, "ty/source/critters/Grasshopper2.cpp"),
            Object(NonMatching, "ty/source/critters/Ibis.cpp"),
            Object(NonMatching, "ty/source/critters/KingFisher.cpp"),
            Object(NonMatching, "ty/source/critters/Moth.cpp"),
            Object(NonMatching, "ty/source/critters/SmallCrab.cpp"),
            Object(NonMatching, "ty/source/critters/Spider.cpp"),
            Object(NonMatching, "ty/source/critters/Turtle.cpp"),
            Object(NonMatching, "ty/source/critters/WaterDragon.cpp"),
            Object(NonMatching, "ty/source/cutscene/LetterBox.cpp"),
            Object(NonMatching, "ty/source/dialogplayer/DialogEffect.cpp"),
            Object(NonMatching, "ty/source/dialogplayer/DialogPlayer.cpp"),
            Object(NonMatching, "ty/source/effects/Avalanche.cpp"),
            Object(Matching, "ty/source/effects/Bubble.cpp"),
            Object(Matching, "ty/source/effects/ChronorangEffects.cpp"),
            Object(Matching, "ty/source/effects/Explosion.cpp"),
            Object(Matching, "ty/source/effects/Flame.cpp"),
            Object(NonMatching, "ty/source/effects/GreySmoke.cpp"),
            Object(NonMatching, "ty/source/effects/Lightning.cpp"),
            Object(NonMatching, "ty/source/effects/Rain.cpp"),
            Object(Matching, "ty/source/effects/RainbowEffect.cpp"),
            Object(NonMatching, "ty/source/effects/SawDust.cpp"),
            Object(NonMatching, "ty/source/effects/SnowBall.cpp"),
            Object(NonMatching, "ty/source/effects/SnowFootEffects.cpp"),
            Object(NonMatching, "ty/source/effects/SpeedboatWake.cpp"),
            Object(NonMatching, "ty/source/effects/Steam.cpp"),
            Object(NonMatching, "ty/source/effects/Teleport.cpp"),
            Object(Matching, "ty/source/effects/Trails.cpp"),
            Object(NonMatching, "ty/source/effects/Waterdrip.cpp"),
            Object(NonMatching, "ty/source/effects/Weather.cpp"),
            Object(NonMatching, "ty/source/enemies/BadBoonie.cpp"),
            Object(NonMatching, "ty/source/enemies/Barracuda.cpp"),
            Object(NonMatching, "ty/source/enemies/Bat.cpp"),
            Object(NonMatching, "ty/source/enemies/BlueTongue.cpp"),
            Object(NonMatching, "ty/source/enemies/BlueTongueRock.cpp"),
            Object(NonMatching, "ty/source/enemies/BossCass.cpp"),
            Object(NonMatching, "ty/source/enemies/bull.cpp"),
            Object(NonMatching, "ty/source/enemies/CaveBat.cpp"),
            Object(NonMatching, "ty/source/enemies/Crikey.cpp"),
            Object(NonMatching, "ty/source/enemies/croc.cpp"),
            Object(NonMatching, "ty/source/enemies/Dennis.cpp"),
            Object(NonMatching, "ty/source/enemies/Eel.cpp"),
            Object(NonMatching, "ty/source/enemies/Enemies.cpp"),
            Object(NonMatching, "ty/source/enemies/EnemySpawner.cpp"),
            Object(NonMatching, "ty/source/enemies/Fluffy.cpp"),
            Object(NonMatching, "ty/source/enemies/FrillBike.cpp"),
            Object(NonMatching, "ty/source/enemies/frilllizard.cpp"),
            Object(NonMatching, "ty/source/enemies/FrillSpeedboat.cpp"),
            Object(NonMatching, "ty/source/enemies/Grouper.cpp"),
            Object(NonMatching, "ty/source/enemies/leech.cpp"),
            Object(NonMatching, "ty/source/enemies/LilNeddy.cpp"),
            Object(NonMatching, "ty/source/enemies/muddie.cpp"),
            Object(NonMatching, "ty/source/enemies/neddy.cpp"),
            Object(NonMatching, "ty/source/enemies/NinjaGecko.cpp"),
            Object(NonMatching, "ty/source/enemies/Rhino.cpp"),
            Object(NonMatching, "ty/source/enemies/RhinoGround.cpp"),
            Object(NonMatching, "ty/source/enemies/Roach.cpp"),
            Object(NonMatching, "ty/source/enemies/rufus.cpp"),
            Object(NonMatching, "ty/source/enemies/ScrubTurkey.cpp"),
            Object(NonMatching, "ty/source/enemies/ScubaFrill.cpp"),
            Object(NonMatching, "ty/source/enemies/ShadowBat.cpp"),
            Object(NonMatching, "ty/source/enemies/Shark.cpp"),
            Object(NonMatching, "ty/source/enemies/Skink.cpp"),
            Object(NonMatching, "ty/source/enemies/Sly.cpp"),
            Object(NonMatching, "ty/source/enemies/SnowRoo.cpp"),
            Object(NonMatching, "ty/source/enemies/SnowSpider.cpp"),
            Object(NonMatching, "SpiderTrapdoor.cpp"),
            Object(NonMatching, "ty/source/enemies/SynkerBlueTongue.cpp"),
            Object(NonMatching, "ty/source/enemies/SynkerFrill.cpp"),
            Object(NonMatching, "ty/source/enemies/Tick.cpp"),
            Object(NonMatching, "ty/source/enemies/TrapdoorSpider.cpp"),
            Object(NonMatching, "ty/source/enemies/WhiteBat.cpp"),
            Object(NonMatching, "ty/source/enemies/Wombat.cpp"),
            Object(NonMatching, "ty/source/enemies/Yabby.cpp"),
            Object(NonMatching, "ty/source/friends/Aurora.cpp"),
            Object(NonMatching, "ty/source/friends/AuroraKid.cpp"),
            Object(NonMatching, "ty/source/friends/BunyipElder.cpp"),
            Object(NonMatching, "ty/source/friends/C3_MiniGame_Rex_Elle.cpp"),
            Object(NonMatching, "ty/source/friends/farlapp.cpp"),
            Object(Matching, "ty/source/friends/Friend.cpp"),
            Object(NonMatching, "ty/source/friends/julius.cpp"),
            Object(NonMatching, "ty/source/friends/KenOath.cpp"),
            Object(NonMatching, "ty/source/friends/Lenny.cpp"),
            Object(NonMatching, "ty/source/friends/maurie.cpp"),
            Object(NonMatching, "ty/source/friends/shazza.cpp"),
            Object(NonMatching, "ty/source/friends/sheila.cpp"),
            Object(NonMatching, "ty/source/frontend/FrontEnd.cpp"),
            Object(NonMatching, "ty/source/frontend/FrontEnd_Credits.cpp"),
            Object(NonMatching, "ty/source/frontend/FrontEnd_Extras.cpp"),
            Object(NonMatching, "ty/source/frontend/FrontEnd_Gallery.cpp"),
            Object(NonMatching, "ty/source/frontend/FrontEnd_LanguageSelect.cpp"),
            Object(NonMatching, "ty/source/frontend/FrontEnd_LoadGame.cpp"),
            Object(NonMatching, "ty/source/frontend/FrontEnd_LoadLevel.cpp"),
            Object(NonMatching, "ty/source/frontend/FrontEnd_MainMenu.cpp"),
            Object(NonMatching, "ty/source/frontend/FrontEnd_Title.cpp"),
            Object(NonMatching, "ty/source/frontend/FrontEnd_Videos.cpp"),
            Object(NonMatching, "ty/source/frontend/PauseScreen.cpp"),
            Object(NonMatching, "ty/source/frontend/PauseScreen_Continue.cpp"),
            Object(NonMatching, "ty/source/frontend/PauseScreen_GameInfo.cpp"),
            Object(NonMatching, "ty/source/frontend/PauseScreen_GameTotals.cpp"),
            Object(NonMatching, "ty/source/frontend/PauseScreen_Map.cpp"),
            Object(NonMatching, "ty/source/frontend/PauseScreen_OptionControls.cpp"),
            Object(NonMatching, "ty/source/frontend/PauseScreen_Options.cpp"),
            Object(NonMatching, "ty/source/frontend/PauseScreen_OptionScreen.cpp"),
            Object(NonMatching, "ty/source/frontend/PauseScreen_OptionSound.cpp"),
            Object(NonMatching, "ty/source/frontend/PauseScreen_RangInfo.cpp"),
            Object(NonMatching, "ty/source/frontend/PauseScreen_SaveGame.cpp"),
            Object(Matching, "ty/source/frontend/ProgressBar.cpp"),
            Object(NonMatching, "ty/source/frontend/StartupScreens.cpp"),
            Object(Matching, "ty/source/frontend/UITools.cpp"),
            Object(NonMatching, "ty/source/minigames/MiniGame_A2.cpp"),
            Object(NonMatching, "ty/source/minigames/MiniGame_B1.cpp"),
            Object(NonMatching, "ty/source/minigames/MiniGame_B3.cpp"),
            Object(NonMatching, "ty/source/minigames/MiniGame_C2.cpp"),
            Object(NonMatching, "ty/source/minigames/MiniGame_C3.cpp"),
            Object(NonMatching, "ty/source/minigames/MiniGame_E4.cpp"),
            Object(NonMatching, "ty/source/minigames/MiniGameObjective.cpp"),
            Object(NonMatching, "ty/source/props/A1_FrillObjective.cpp"),
            Object(Matching, "ty/source/props/AnimatingProp.cpp"),
            Object(Matching, "ty/source/props/Aquarang.cpp"),
            Object(NonMatching, "ty/source/props/BeachHut.cpp"),
            Object(NonMatching, "ty/source/props/BonusPickup.cpp"),
            Object(NonMatching, "ty/source/props/Bouncer.cpp"),
            Object(NonMatching, "ty/source/props/BubbleSpawner.cpp"),
            Object(Matching, "ty/source/props/BunyipStone.cpp"),
            Object(NonMatching, "ty/source/props/Burnable.cpp"),
            Object(NonMatching, "ty/source/props/Burner.cpp"),
            Object(NonMatching, "ty/source/props/BurningTree.cpp"),
            Object(NonMatching, "ty/source/props/BuzzSaw2.cpp"),
            Object(NonMatching, "ty/source/props/C1Water.cpp"),
            Object(NonMatching, "ty/source/props/C3_Chest.cpp"),
            Object(NonMatching, "ty/source/props/C3_Ringo.cpp"),
            Object(NonMatching, "ty/source/props/CableCar.cpp"),
            Object(NonMatching, "ty/source/props/Coconut.cpp"),
            Object(Matching, "ty/source/props/Collapsible.cpp"),
            Object(NonMatching, "ty/source/props/CrikeyProps.cpp"),
            Object(Matching, "ty/source/props/D1_Prop.cpp"),
            Object(NonMatching, "ty/source/props/Damageable.cpp"),
            Object(Matching, "ty/source/props/DDACheckpoint.cpp"),
            Object(NonMatching, "ty/source/props/E4_Door.cpp"),
            Object(NonMatching, "ty/source/props/E4_FlameThrower.cpp"),
            Object(NonMatching, "ty/source/props/Elevator.cpp"),
            Object(NonMatching, "ty/source/props/FallGuideProp.cpp"),
            Object(NonMatching, "ty/source/props/FireDrum.cpp"),
            Object(NonMatching, "ty/source/props/FlamingLog.cpp"),
            Object(NonMatching, "ty/source/props/Furnace.cpp"),
            Object(NonMatching, "ty/source/props/GasJet.cpp"),
            Object(Matching, "ty/source/props/gem.cpp"),
            Object(NonMatching, "ty/source/props/Generator.cpp"),
            Object(NonMatching, "ty/source/props/Geyser.cpp"),
            Object(NonMatching, "ty/source/props/IceCoolant.cpp"),
            Object(NonMatching, "ty/source/props/IceRamp.cpp"),
            Object(NonMatching, "ty/source/props/JuliusMachine.cpp"),
            Object(NonMatching, "ty/source/props/KoalaKid.cpp"),
            Object(NonMatching, "ty/source/props/Lava.cpp"),
            Object(NonMatching, "ty/source/props/LavaBurner.cpp"),
            Object(NonMatching, "ty/source/props/LilyPad.cpp"),
            Object(NonMatching, "ty/source/props/Log.cpp"),
            Object(NonMatching, "ty/source/props/MobilePlatform.cpp"),
            Object(NonMatching, "ty/source/props/MorayEel.cpp"),
            Object(NonMatching, "ty/source/props/MusicalIcicle.cpp"),
            Object(NonMatching, "ty/source/props/OpalCollector.cpp"),
            Object(NonMatching, "ty/source/props/PickupCounter.cpp"),
            Object(NonMatching, "ty/source/props/PicnicBasket.cpp"),
            Object(Matching, "ty/source/props/Picture.cpp"),
            Object(NonMatching, "ty/source/props/PipePart.cpp"),
            Object(Matching, "ty/source/props/Platform.cpp"),
            Object(NonMatching, "ty/source/props/PontoonRope.cpp"),
            Object(Matching, "ty/source/props/Projectile.cpp"),
            Object(NonMatching, "ty/source/props/Props.cpp"),
            Object(Matching, "ty/source/props/RangStone.cpp"),
            Object(NonMatching, "ty/source/props/Rex.cpp"),
            Object(NonMatching, "ty/source/props/Sentinel.cpp"),
            Object(NonMatching, "ty/source/props/ShadowBatProps.cpp"),
            Object(NonMatching, "ty/source/props/SharkCage.cpp"),
            Object(Matching, "ty/source/props/Shatterable.cpp"),
            Object(NonMatching, "ty/source/props/SnowPile.cpp"),
            Object(Matching, "ty/source/props/SoundProp.cpp"),
            Object(Matching, "ty/source/props/SpawnPoint.cpp"),
            Object(NonMatching, "ty/source/props/SpiderWeb.cpp"),
            Object(NonMatching, "ty/source/props/SpinningLog.cpp"),
            Object(NonMatching, "ty/source/props/Stalactite.cpp"),
            Object(Matching, "ty/source/props/StaticProp.cpp"),
            Object(Matching, "ty/source/props/Talisman.cpp"),
            Object(NonMatching, "ty/source/props/Teleporter.cpp"),
            Object(NonMatching, "ty/source/props/ThinIceProp.cpp"),
            Object(NonMatching, "ty/source/props/ThunderEggCollector.cpp"),
            Object(NonMatching, "ty/source/props/TreasureChest.cpp"),
            Object(NonMatching, "ty/source/props/TriggerProp.cpp"),
            Object(NonMatching, "ty/source/props/TumbleWeed.cpp"),
            Object(NonMatching, "ty/source/props/TurningPlatform.cpp"),
            Object(NonMatching, "ty/source/props/WarpFlower.cpp"),
            Object(NonMatching, "ty/source/props/WaterPipe.cpp"),
            Object(Matching, "ty/source/props/WaterVolume.cpp"),
            Object(NonMatching, "ty/source/props/WaterWheel.cpp"),
            Object(Matching, "ty/source/props/WeatherProp.cpp"),
            Object(NonMatching, "ty/source/props/WhirlyWind.cpp"),
            Object(NonMatching, "ty/source/props/YabbyStalactite.cpp"),
            Object(Matching, "ty/source/props/Z1_RainbowScales.cpp"),
            Object(NonMatching, "ty/source/props/ZoneBlockerProp.cpp"),
            Object(NonMatching, "ty/source/anthill.cpp"),
            Object(NonMatching, "ty/source/AS_AirStates.cpp"),
            Object(NonMatching, "ty/source/AS_BiteStates.cpp"),
            Object(NonMatching, "ty/source/AS_LandActionStates.cpp"),
            Object(NonMatching, "ty/source/AS_LandStates.cpp"),
            Object(NonMatching, "ty/source/AS_MiscStates.cpp"),
            Object(NonMatching, "ty/source/AS_SlideStates.cpp"),
            Object(NonMatching, "ty/source/AS_UnderWaterStates.cpp"),
            Object(NonMatching, "ty/source/AS_WaterStates.cpp"),
            Object(NonMatching, "ty/source/barbedWire.cpp"),
            Object(NonMatching, "ty/source/BezierPathFollower.cpp"),
            Object(NonMatching, "ty/source/bilby.cpp"),
            Object(Matching, "ty/source/boomerang.cpp"),
            Object(NonMatching, "ty/source/BoomerangHud.cpp"),
            Object(Matching, "ty/source/boomerangManager.cpp"),
            Object(NonMatching, "ty/source/BouncingBoulder.cpp"),
            Object(Matching, "ty/source/BoundingRegion.cpp"),
            Object(Matching, "ty/source/bunyip.cpp"),
            Object(NonMatching, "ty/source/BushPig.cpp"),
            Object(NonMatching, "ty/source/checkpoint.cpp"),
            Object(Matching, "ty/source/CollisionObject.cpp"),
            Object(Matching, "ty/source/controlval.cpp"),
            Object(NonMatching, "ty/source/crate.cpp"),
            Object(Matching, "ty/source/DataVal.cpp"),
            Object(Matching, "ty/source/DDA.cpp"),
            Object(NonMatching, "ty/source/draw.cpp"),
            Object(NonMatching, "ty/source/Drum.cpp"),
            Object(NonMatching, "ty/source/DustTrail.cpp"),
            Object(NonMatching, "ty/source/elle.cpp"),
            Object(NonMatching, "ty/source/emu.cpp"),
            Object(Matching, "ty/source/ExtendedAnalogControl.cpp"),
            Object(NonMatching, "ty/source/finishline.cpp"),
            Object(Matching, "ty/source/GameData.cpp"),
            Object(Matching, "ty/source/GameObject.cpp"),
            Object(Matching, "ty/source/GameObjectManager.cpp"),
            Object(NonMatching, "ty/source/Gate.cpp"),
            Object(Matching, "ty/source/global.cpp"),
            Object(NonMatching, "ty/source/guidebarrel.cpp"),
            Object(Matching, "ty/source/GuideParticle.cpp"),
            Object(Matching, "ty/source/heatflare.cpp"),
            Object(NonMatching, "ty/source/Hud.cpp"),
            Object(NonMatching, "ty/source/iceblock.cpp"),
            Object(NonMatching, "ty/source/jeep.cpp"),
            Object(Matching, "ty/source/Kinematics.cpp"),
            Object(NonMatching, "ty/source/lasso.cpp"),
            Object(Matching, "ty/source/LensFlare.cpp"),
            Object(Matching, "ty/source/LevelObjective.cpp"),
            Object(Matching, "ty/source/LineOfSight.cpp"),
            Object(NonMatching, "ty/source/LoadLevel.cpp"),
            Object(NonMatching, "ty/source/main.cpp"),
            Object(Matching, "ty/source/MessageMap.cpp"),
            Object(Matching, "ty/source/Messages.cpp"),
            Object(NonMatching, "ty/source/MetalSpikes.cpp"),
            Object(Matching, "ty/source/mist.cpp"),
            Object(Matching, "ty/source/NodeOverride.cpp"),
            Object(NonMatching, "ty/source/particleEffects.cpp"),
            Object(NonMatching, "ty/source/ParticleEmitter.cpp"),
            Object(NonMatching, "ty/source/particleengine.cpp"),
            Object(Matching, "ty/source/Path.cpp"),
            Object(NonMatching, "ty/source/Pontoon.cpp"),
            Object(NonMatching, "ty/source/portal.cpp"),
            Object(Matching, "ty/source/quadratic.cpp"),
            Object(Matching, "ty/source/RangeCheck.cpp"),
            Object(NonMatching, "ty/source/reeds.cpp"),
            Object(NonMatching, "ty/source/Reflection.cpp"),
            Object(Matching, "ty/source/renderTexture.cpp"),
            Object(Matching, "ty/source/Script.cpp"),
            Object(NonMatching, "ty/source/SeaMine.cpp"),
            Object(Matching, "ty/source/setup.cpp"),
            Object(Matching, "ty/source/Shadow.cpp"),
            Object(NonMatching, "ty/source/Shatter.cpp"),
            Object(NonMatching, "ty/source/shears.cpp"),
            Object(NonMatching, "ty/source/SignPost.cpp"),
            Object(NonMatching, "ty/source/soundbank.cpp"),
            Object(NonMatching, "ty/source/SpecialPickup.cpp"),
            Object(NonMatching, "ty/source/SpikeyIce.cpp"),
            Object(Matching, "ty/source/Spline.cpp"),
            Object(NonMatching, "ty/source/staticSpikes.cpp"),
            Object(NonMatching, "ty/source/stopwatch.cpp"),
            Object(NonMatching, "ty/source/target.cpp"),
            Object(Matching, "ty/source/tools.cpp"),
            Object(Matching, "ty/source/Torch.cpp"),
            Object(NonMatching, "ty/source/Ty.cpp"),
            Object(NonMatching, "ty/source/TyAnimTables.cpp"),
            Object(NonMatching, "ty/source/TyAttributes.cpp"),
            Object(NonMatching, "ty/source/TyCollisions.cpp"),
            Object(NonMatching, "ty/source/TyHealth.cpp"),
            Object(NonMatching, "ty/source/TyMemCard.cpp"),
            Object(NonMatching, "ty/source/TyMemCardQuery.cpp"),
            Object(NonMatching, "ty/source/underWaterEffects.cpp"),
            Object(Matching, "ty/source/UserInterface.cpp"),
            Object(NonMatching, "ty/source/wake.cpp"),
            Object(NonMatching, "ty/source/Waterfall.cpp"),
            Object(NonMatching, "ty/source/WaterfallBase.cpp"),
            Object(NonMatching, "ty/source/WaterfallPoint.cpp"),
            Object(NonMatching, "ty/source/WaterfallSpray.cpp"),
            Object(NonMatching, "ty/source/WaterfallStream.cpp"),
            Object(NonMatching, "ty/source/WaterSlide.cpp"),
            Object(NonMatching, "ty/source/watertank.cpp"),
            Object(NonMatching, "ty/source/WeatherTypes.cpp"),
            Object(NonMatching, "ty/source/windmill.cpp"),
            Object(Matching, "ty/source/WobbleTexture.cpp"),
            # Object(NonMatching, "fileCache.c"),
            # Object(NonMatching, "List.c"),
            # Object(NonMatching, "string1/string.c"),
            # Object(NonMatching, "Runtime/abort_exit.c"),
            # Object(NonMatching, "Runtime/alloc.c"),
            # Object(NonMatching, "Runtime/errno.c"),
            # Object(NonMatching, "Runtime/ansi_files.c"),
            # Object(NonMatching, "Runtime/ansi_fp.c"),
            # Object(NonMatching, "Runtime/arith.c"),
            # Object(NonMatching, "Runtime/buffer_io.c"),
            # Object(NonMatching, "Runtime/ctype.c"),
            # Object(NonMatching, "Runtime/locale.c"),
            # Object(NonMatching, "Runtime/direct_io.c"),
            # Object(NonMatching, "Runtime/file_io.c"),
            # Object(NonMatching, "Runtime/FILE_POS.C"),
            # Object(NonMatching, "Runtime/mbstring.c"),
            # Object(NonMatching, "Runtime/mem.c"),
            # Object(NonMatching, "Runtime/mem_funcs.c"),
            # Object(NonMatching, "Runtime/misc_io.c"),
            # Object(NonMatching, "Runtime/printf.c"),
            # Object(NonMatching, "Runtime/qsort.c"),
            # Object(NonMatching, "Runtime/rand.c"),
            # Object(NonMatching, "scanf.c"),
            # Object(NonMatching, "Runtime/string.c"),
            # Object(NonMatching, "Runtime/float.c"),
            # Object(NonMatching, "Runtime/strtold.c"),
            # Object(NonMatching, "Runtime/strtoul.c"),
            # Object(NonMatching, "Runtime/uart_console_io.c"),
            # Object(NonMatching, "Runtime/wchar_io.c"),
            # Object(NonMatching, "Runtime/e_acos.c"),
            # Object(NonMatching, "Runtime/e_asin.c"),
            # Object(NonMatching, "Runtime/e_atan2.c"),
            # Object(NonMatching, "Runtime/e_fmod.c"),
            # Object(NonMatching, "Runtime/e_log.c"),
            # Object(NonMatching, "Runtime/e_log10.c"),
            # Object(NonMatching, "Runtime/e_pow.c"),
            # Object(NonMatching, "Runtime/e_rem_pio2.c"),
            # Object(NonMatching, "Runtime/k_rem_pio2.c"),
            # Object(NonMatching, "Runtime/k_tan.c"),
            # Object(NonMatching, "s_atan.c"),
            # Object(NonMatching, "Runtime/s_copysign.c"),
            # Object(NonMatching, "Runtime/s_floor.c"),
            # Object(NonMatching, "Runtime/s_frexp.c"),
            # Object(NonMatching, "Runtime/s_ldexp.c"),
            # Object(NonMatching, "Runtime/s_modf.c"),
            # Object(NonMatching, "Runtime/s_nextafter.c"),
            # Object(NonMatching, "Runtime/s_tan.c"),
            # Object(NonMatching, "Runtime/w_acos.c"),
            # Object(NonMatching, "Runtime/w_asin.c"),
            # Object(NonMatching, "Runtime/w_atan2.c"),
            # Object(NonMatching, "Runtime/w_fmod.c"),
            # Object(NonMatching, "Runtime/w_log.c"),
            # Object(NonMatching, "Runtime/w_log10.c"),
            # Object(NonMatching, "Runtime/w_pow.c"),
            # Object(NonMatching, "Runtime/__va_arg.c"),
            # Object(NonMatching, "Runtime/global_destructor_chain.c"),
            # Object(NonMatching, "Runtime/NMWException.cp"),
            # Object(NonMatching, "Runtime/ptmf.c"),
            # Object(NonMatching, "runtime.c"),
            # Object(NonMatching, "Runtime/__init_cpp_exceptions.cpp"),
            # Object(NonMatching, "Runtime/Gecko_ExceptionPPC.cp"),
            # Object(NonMatching, "GCN_mem_alloc.c"),
            # Object(NonMatching, "mainloop.c"),
            # Object(NonMatching, "nubevent.c"),
            # Object(NonMatching, "nubinit.c"),
            # Object(NonMatching, "msg.c"),
            # Object(NonMatching, "msgbuf.c"),
            # Object(NonMatching, "serpoll.c"),
            # Object(NonMatching, "usr_put.c"),
            # Object(NonMatching, "dispatch.c"),
            # Object(NonMatching, "msghndlr.c"),
            # Object(NonMatching, "support.c"),
            # Object(NonMatching, "mutex_TRK.c"),
            # Object(NonMatching, "notify.c"),
            # Object(NonMatching, "flush_cache.c"),
            # Object(NonMatching, "mem_TRK.c"),
            # Object(NonMatching, "targimpl.c"),
            # Object(NonMatching, "dolphin_trk.c"),
            # Object(NonMatching, "mpc_7xx_603e.c"),
            # Object(NonMatching, "main_TRK.c"),
            # Object(NonMatching, "dolphin_trk_glue.c"),
            # Object(NonMatching, "targcont.c"),
            # Object(NonMatching, "target_options.c"),
            # Object(NonMatching, "mslsupp.c"),
            Object(Matching, "common/Source/Animation.cpp"),
            Object(Matching, "common/Source/Camera.cpp"),
            Object(Matching, "common/Source/Collision.cpp"),
            Object(Matching, "common/Source/Crc.cpp"),
            Object(Matching, "common/Source/DirectLight.cpp"),
            Object(Matching, "common/Source/FileSys.cpp"),
            Object(Matching, "common/Source/Font.cpp"),
            Object(Matching, "common/Source/Heap.cpp"),
            Object(Matching, "common/Source/KromeIni.cpp"),
            Object(Matching, "common/Source/Matrix.cpp"),
            Object(Matching, "common/Source/MKAnimScript.cpp"),
            Object(Matching, "common/Source/MKPackage.cpp"),
            Object(Matching, "common/Source/MKParticleGen.cpp"),
            Object(Matching, "common/Source/MKRumble.cpp"),
            Object(Matching, "common/Source/MKSceneManager.cpp"),
            Object(Matching, "common/Source/Model.cpp"),
            Object(Matching, "common/Source/ParticleSystem.cpp"),
            Object(Matching, "common/Source/ParticleSystemManager.cpp"),
            Object(Matching, "common/Source/QuatRotation.cpp"),
            Object(Matching, "common/Source/StdMath.cpp"),
            Object(Matching, "common/Source/Str.cpp"),
            Object(Matching, "common/Source/Timer.cpp"),
            Object(Matching, "common/Source/Translation.cpp"),
            Object(Matching, "common/Source/Utils.cpp"),
            Object(Matching, "common/Source/Vector.cpp"),
            Object(Matching, "common/gc/demoinit.cpp"),
            Object(Matching, "common/gc/DiscErrors.cpp"),
            Object(Matching, "common/gc/File.cpp"),
            Object(Matching, "common/gc/Grass_GC.cpp"),
            Object(Matching, "common/gc/Input_GC.cpp"),
            Object(Matching, "common/gc/Material_GC.cpp"),
            Object(NonMatching, "common/gc/MKMemoryCard.cpp"),
            Object(Matching, "common/gc/MKShadow_GC.cpp"),
            Object(Matching, "common/gc/ModelGC.cpp"),
            Object(NonMatching, "common/gc/Sound.cpp"),
            Object(Matching, "common/gc/system_extras.cpp"),
            Object(Matching, "common/gc/System_GC.cpp"),
            Object(Matching, "common/gc/Texture.cpp"),
            Object(Matching, "common/gc/THPAudioDecode.c"),
            Object(NonMatching, "THPPlayer.c"),
            Object(NonMatching, "THPRead.c"),
            Object(NonMatching, "THPVideoDecode.c"),
            Object(Matching, "common/gc/Video.cpp"),
            Object(Matching, "common/gc/View_GC.cpp"),
            Object(Matching, "common/gc/Water_GC.cpp"),
            Object(NonMatching, "xFont.cpp"),
            Object(NonMatching, "xfontdata.cpp"),
            Object(Matching, "common/gc/Blitter.cpp"),
            Object(Matching, "common/Debug.cpp"),
        ],
    ),
    DolphinLib(
        "ai",
        [
            Object(NonMatching, "Dolphin/ai.c"),
        ],
    ),
    DolphinLib(
        "ar",
        [
            Object(NonMatching, "Dolphin/ar/ar.c"),
        ],
    ),
    DolphinLib(
        "ax",
        [
            Object(NonMatching, "Dolphin/ax/AX.c"),
            Object(NonMatching, "Dolphin/ax/AXAlloc.c"),
            Object(NonMatching, "Dolphin/ax/AXAux.c"),
            Object(NonMatching, "Dolphin/ax/AXCL.c"),
            Object(NonMatching, "Dolphin/ax/AXOut.c"),
            Object(NonMatching, "Dolphin/ax/AXSPB.c"),
            Object(NonMatching, "Dolphin/ax/AXVPB.c"),
            Object(NonMatching, "Dolphin/ax/AXComp.c"),
            Object(NonMatching, "Dolphin/ax/DSPCode.c"),
            Object(NonMatching, "Dolphin/ax/AXProf.c"),
        ]
    ),
    DolphinLib(
        "base",
        [
            Object(NonMatching, "Dolphin/PPCArch.c"),
        ],
    ),
    DolphinLib(
        "db",
        [
            Object(NonMatching, "Dolphin/db.c"),
        ],
    ),
    DolphinLib(
        "dsp",
        [
            Object(NonMatching, "Dolphin/dsp/dsp.c"),
            Object(NonMatching, "Dolphin/dsp/dsp_debug.c"),
            Object(NonMatching, "Dolphin/dsp/dsp_task.c"),
        ],
    ),
    DolphinLib(
        "dvd",
        [
            Object(NonMatching, "Dolphin/dvd/dvdlow.c"),
            Object(NonMatching, "Dolphin/dvd/dvdfs.c"),
            Object(NonMatching, "Dolphin/dvd/dvd.c"),
            Object(NonMatching, "Dolphin/dvd/dvdqueue.c"),
            Object(NonMatching, "Dolphin/dvd/dvderror.c"),
            Object(NonMatching, "Dolphin/dvd/fstload.c"),
        ],
    ),
    DolphinLib(
        "gx",
        [
            Object(NonMatching, "Dolphin/gx/GXInit.c"),
            Object(NonMatching, "Dolphin/gx/GXFifo.c"),
            Object(NonMatching, "Dolphin/gx/GXAttr.c"),
            Object(NonMatching, "Dolphin/gx/GXMisc.c"),
            Object(NonMatching, "Dolphin/gx/GXGeometry.c"),
            Object(NonMatching, "Dolphin/gx/GXFrameBuf.c"),
            Object(NonMatching, "Dolphin/gx/GXLight.c"),
            Object(NonMatching, "Dolphin/gx/GXTexture.c"),
            Object(NonMatching, "Dolphin/gx/GXBump.c"),
            Object(NonMatching, "Dolphin/gx/GXTev.c"),
            Object(NonMatching, "Dolphin/gx/GXPixel.c"),
            Object(NonMatching, "Dolphin/gx/GXStubs.c"),
            Object(NonMatching, "Dolphin/gx/GXDisplayList.c"),
            Object(NonMatching, "Dolphin/gx/GXTransform.c"),
            Object(NonMatching, "Dolphin/gx/GXPerf.c"),
        ],
    ),
    DolphinLib(
        "tex",
        [
            Object(NonMatching, "Dolphin/tex/texPalette.c"),
        ],
    ),
    DolphinLib(
        "os",
        [
            Object(Matching, "Dolphin/os/__start.c"),
            Object(Matching, "Dolphin/os/OS.c"),
            Object(NonMatching, "Dolphin/os/OSAlarm.c"),
            Object(NonMatching, "Dolphin/os/OSAlloc.c"),
            Object(Matching, "Dolphin/os/OSArena.c"),
            Object(Matching, "Dolphin/os/OSAudioSystem.c"),
            Object(NonMatching, "Dolphin/os/OSCache.c"),
            Object(Matching, "Dolphin/os/OSContext.c"),
            Object(Matching, "Dolphin/os/OSError.c"),
            Object(NonMatching, "Dolphin/os/OSFont.c"),
            Object(Matching, "Dolphin/os/OSInterrupt.c"),
            Object(Matching, "Dolphin/os/OSLink.c"),
            Object(NonMatching, "Dolphin/os/OSMessage.c"),
            Object(NonMatching, "Dolphin/os/OSMemory.c"),
            Object(NonMatching, "Dolphin/os/OSMutex.c"),
            Object(NonMatching, "Dolphin/os/OSReboot.c"),
            Object(Matching, "Dolphin/os/OSReset.c"),
            Object(NonMatching, "Dolphin/os/OSResetSW.c"),
            Object(Matching, "Dolphin/os/OSRtc.c"),
            Object(NonMatching, "Dolphin/os/OSSync.c"),
            Object(Matching, "Dolphin/os/OSThread.c"),
            Object(NonMatching, "Dolphin/os/OSTime.c"),
            Object(Matching, "Dolphin/os/__ppc_eabi_init.cpp"),
        ]
    ),
    DolphinLib(
        "pad",
        [
            Object(NonMatching, "Dolphin/pad/Pad.c"),
        ],
    ),
    DolphinLib(
        "vi",
        [
            Object(NonMatching, "Dolphin/vi.c"),
        ],
    ),
    DolphinLib(
        "dtk",
        [
            Object(Matching, "Dolphin/dtk.c"),
        ]
    ),
    DolphinLib(
        "card",
        [
            Object(Matching, "Dolphin/card/CARDBios.c"),
            Object(Matching, "Dolphin/card/CARDUnlock.c"),
            Object(Matching, "Dolphin/card/CARDRdwr.c"),
            Object(Matching, "Dolphin/card/CARDBlock.c"),
            Object(Matching, "Dolphin/card/CARDDir.c"),
            Object(NonMatching, "Dolphin/card/CARDCheck.c"),
            Object(Matching, "Dolphin/card/CARDMount.c"),
            Object(NonMatching, "Dolphin/card/CARDFormat.c"),
            Object(NonMatching, "Dolphin/card/CARDOpen.c"),
            Object(NonMatching, "Dolphin/card/CARDCreate.c"),
            Object(NonMatching, "Dolphin/card/CARDRead.c"),
            Object(NonMatching, "Dolphin/card/CARDWrite.c"),
            Object(NonMatching, "Dolphin/card/CARDDelete.c"),
            Object(NonMatching, "Dolphin/card/CARDStat.c"),
            Object(NonMatching, "Dolphin/card/CARDNet.c"),
        ]
    ),
    DolphinLib(
        "si",
        [
            Object(NonMatching, "Dolphin/si/SIBios.c"),
            Object(NonMatching, "Dolphin/si/SISamplingRate.c"),
        ],
    ),
    DolphinLib(
        "amcstubs",
        [
            Object(NonMatching, "Dolphin/amcstubs/AmcExi2Stubs.c"),
        ]
    ),
    DolphinLib(
        "odenotstub",
        [
            Object(NonMatching, "Dolphin/odenotstub/odenotstub.c"),
        ]
    ),
    DolphinLib(
        "mtx",
        [
            Object(NonMatching, "Dolphin/mtx/mtx.c"),
            Object(NonMatching, "Dolphin/mtx/mtx44.c"),
        ]
    ),
    DolphinLib(
        "OdemuExi2",
        [
            Object(NonMatching, "DebuggerDriver.c"),
        ],
    ),
    DolphinLib(
        "axart",
        [
            Object(NonMatching, "Dolphin/axart/axart.c"),
            Object(NonMatching, "Dolphin/axart/axartsound.c"),
            Object(NonMatching, "Dolphin/axart/axartcents.c"),
            Object(NonMatching, "Dolphin/axart/axartenv.c"),
            Object(NonMatching, "Dolphin/axart/axartlfo.c"),
            Object(NonMatching, "Dolphin/axart/axart3d.c"),
        ]
    ),
    DolphinLib(
        "mix",
        [
            Object(NonMatching, "Dolphin/mix/mix.c"),
        ]
    ),
    DolphinLib(
        "exi",
        [
            Object(NonMatching, "Dolphin/exi/EXIBios.c"),
            Object(NonMatching, "Dolphin/exi/EXIUart.c"),
        ],
    ),
    DolphinLib(
        "thp",
        [
            Object(Matching, "Dolphin/thp/THPDec.c", mw_version="GC/1.2.5"),
            Object(NonMatching, "Dolphin/thp/THPAudio.c"),
        ],
    ),
    {
        "lib": "Runtime.PPCEABI.H",
        "mw_version": "GC/1.3",
        "cflags": cflags_runtime,
        "progress_category": "sdk",
        "shift_jis": False,
        "objects": [
            Object(Matching, "Runtime/__mem.c", mw_version="GC/1.3.2"),
            Object(NonMatching, "Runtime/__va_arg.c"),
            Object(Matching, "Runtime/global_destructor_chain.c"),
            Object(Matching, "Runtime/NMWException.cp"),
            Object(Matching, "Runtime/ptmf.c"),
            Object(NonMatching, "Runtime/runtime.c"),
            Object(Matching, "Runtime/__init_cpp_exceptions.cpp"),
            Object(Matching, "Runtime/Gecko_ExceptionPPC.cp"),
            Object(NonMatching, "Runtime/GCN_mem_alloc.c"),
            Object(Matching, "Runtime/abort_exit.c"),
            Object(NonMatching, "Runtime/alloc.c"),
            Object(NonMatching, "Runtime/ansi_files.c"),
            Object(NonMatching, "Runtime/ansi_fp.c"),
            Object(NonMatching, "Runtime/arith.c"),
            Object(NonMatching, "Runtime/buffer_io.c"),
            Object(NonMatching, "Runtime/ctype.c"),
            Object(NonMatching, "Runtime/locale.c"),
            Object(NonMatching, "Runtime/direct_io.c"),
            Object(NonMatching, "Runtime/file_io.c"),
            Object(NonMatching, "Runtime/errno.c"),
            Object(NonMatching, "Runtime/FILE_POS.C"),
            Object(NonMatching, "Runtime/mbstring.c"),
            Object(NonMatching, "Runtime/mem.c"),
            Object(NonMatching, "Runtime/mem_funcs.c"),
            Object(NonMatching, "Runtime/misc_io.c"),
            Object(NonMatching, "Runtime/printf.c"),
            Object(NonMatching, "Runtime/qsort.c"),
            Object(Matching, "Runtime/rand.c"),
            Object(NonMatching, "Runtime/scanf.c"),
            Object(NonMatching, "Runtime/string.c"),
            Object(NonMatching, "Runtime/float.c"),
            Object(NonMatching, "Runtime/strtold.c"),
            Object(NonMatching, "Runtime/uart_console_io.c"),
            Object(NonMatching, "Runtime/wchar_io.c"),
            Object(NonMatching, "Runtime/e_acos.c"),
            Object(NonMatching, "Runtime/e_asin.c"),
            Object(NonMatching, "Runtime/e_atan2.c"),
            Object(NonMatching, "Runtime/e_exp.c"),  
            Object(NonMatching, "Runtime/e_fmod.c"),
            Object(NonMatching, "Runtime/e_log.c"),
            Object(NonMatching, "Runtime/e_pow.c"),  
            Object(NonMatching, "Runtime/e_rem_pio2.c"),
            Object(NonMatching, "Runtime/k_cos.c"),
            Object(NonMatching, "Runtime/k_rem_pio2.c"),
            Object(NonMatching, "Runtime/k_sin.c"),
            Object(NonMatching, "Runtime/k_tan.c"),
            Object(NonMatching, "Runtime/s_atan.c"), 
            Object(NonMatching, "Runtime/s_copysign.c"),
            Object(NonMatching, "Runtime/s_cos.c"),
            Object(NonMatching, "Runtime/s_floor.c"),
            Object(NonMatching, "Runtime/s_frexp.c"),
            Object(NonMatching, "Runtime/s_ldexp.c"),
            Object(NonMatching, "Runtime/s_modf.c"),
            Object(NonMatching, "Runtime/s_nextafter.c"),
            Object(NonMatching, "Runtime/s_sin.c"),
            Object(NonMatching, "Runtime/s_tan.c"),
            Object(NonMatching, "Runtime/w_acos.c"),
            Object(NonMatching, "Runtime/w_asin.c"),
            Object(NonMatching, "Runtime/w_atan2.c"),
            Object(NonMatching, "Runtime/w_exp.c"),
            Object(NonMatching, "Runtime/w_fmod.c"),
            Object(NonMatching, "Runtime/w_log.c"),
            Object(NonMatching, "Runtime/w_pow.c"),
        ],
    },
]


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects

# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("game", "Game Code"),
    ProgressCategory("sdk", "SDK Code"),
]
config.progress_each_module = args.verbose

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress and write progress.json
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
