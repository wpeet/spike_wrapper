import json
from systemrdl import RDLCompiler

def export_rdl_to_json(input_file, output_file):
    # 1. Compile the SystemRDL file
    rdlc = RDLCompiler()
    rdlc.compile_file(input_file)
    root = rdlc.elaborate()

    # 2. Extract properties to dictionary (Custom recursive traversal)
    def serialize_node(node):
        result = {
            "name": node.get_property("name") or node.inst_name,
            "type": node.__class__.__name__,
        }
        if node.is_reg:
            result.update({
                "address_offset": hex(node.address_offset),
                "reset": hex(node.get_property("reset") or 0),
            })
        elif hasattr(node, "children"):
            result["children"] = [serialize_node(child) for child in node.children]
        return result

    # 3. Write to JSON
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(serialize_node(root), f, indent=4)

export_rdl_to_json("my_registers.rdl", "registers.json")

