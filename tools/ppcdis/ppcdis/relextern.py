"""
Rel external label preprocessor
"""

from typing import List

from .binarybase import SectionType
from .binaryrel import RelReader, RelType
from .symbols import LabelManager, LabelType

def label_rel_externs(dest: LabelManager, rel: RelReader):
    for rlc in rel.relocs:
        # Skip local relocs
        if rlc.target_module == rel.module_id:
            continue
        
        # Skip non-reference relocs
        if rlc.t in [RelType.RVL_NONE, RelType.RVL_SECT, RelType.RVL_STOP]:
            continue
        
        # Check known reloc type
        assert rlc.t in [RelType.ADDR32, RelType.ADDR16_LO, RelType.ADDR16_HA, RelType.REL24], \
            f"Unsupported relocation type {rlc.t}"
        
        # Get target
        target = rel.get_reloc_target(rlc)

        # Set type
        if rel.find_section_containing(target).type == SectionType.TEXT:
            dest.set_type(target, LabelType.FUNCTION)
        else:
            dest.set_type(target, LabelType.DATA)

def dump_rel_externs(path: str, rels: List[RelReader]):
    # Get labels
    labels = LabelManager()
    for rel in rels:
        label_rel_externs(labels, rel)
    
    # Output
    labels.output(path)
