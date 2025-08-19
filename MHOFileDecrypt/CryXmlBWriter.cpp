#include "CryXmlBWriter.h"
#include "tinyxml2.h"
#include "Utils.h"

namespace CryXMLB {

    struct cry_xml_node_t {
        int32_t name_offset;     // offset into string blob
        int32_t content_offset;  // offset into string blob (text node) or -1
        int16_t attribute_count;
        int16_t child_count;
        int32_t parent_id;       // -1 if none
        int32_t first_attr_idx;  // index into attr table, or -1
        int32_t first_child_idx; // index into node table, or -1
        int32_t reserved;        // usually 0
    };

    struct cry_xml_ref_t {
        int32_t name_offset;     // offset into string blob
        int32_t value_offset;    // offset into string blob
    };

    struct cry_xml_value_t {
        int32_t offset;          // offset into string blob
        char* value;           // (not written literally — this is an in-memory helper, 
        // on disk you only store the string once in blob)
    };

    struct StringPool {
        std::unordered_map<std::string, int32_t> offs;
        std::vector<uint8_t> blob;

        int32_t add(const std::string& s) {
            auto it = offs.find(s);
            if (it != offs.end()) return it->second;
            int32_t off = (int32_t)blob.size();
            blob.insert(blob.end(), s.begin(), s.end());
            blob.push_back(0);
            offs[s] = off;
            return off;
        }
    };

    struct BuiltAttr {
        int32_t name_off;
        int32_t value_off;
    };

    struct BuiltNode {
        cry_xml_node_t n;
    };


    bool ConvertXmlToCryXmlb(const std::string& xmlPath, const std::string& output) {
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(xmlPath.c_str()) != tinyxml2::XML_SUCCESS) return false;

        auto* root = doc.RootElement();
        if (!root) return false;

        StringPool sp;
        std::vector<BuiltNode> nodes;
        std::vector<BuiltAttr> attrs;
        std::vector<std::tuple<int32_t, int32_t>> child_table;

        std::function<int32_t(tinyxml2::XMLElement*, int32_t)> buildNode =
            [&](tinyxml2::XMLElement* e, int32_t parentId) -> int32_t {
            cry_xml_node_t n{};
            auto name = e->Name();
            n.name_offset = sp.add(e->Name());
            sp.add("");
            n.content_offset = 8;
            n.attribute_count = 0;
            n.child_count = 0;
            n.parent_id = parentId;
            n.first_attr_idx = -1;
            n.first_child_idx = 0;
            n.reserved = 0;

            int32_t myIdx = (int32_t)nodes.size();
            nodes.push_back({ n });

            // attributes
            int32_t firstAttr = (int32_t)attrs.size();
            int countAttr = 0;
            for (auto* a = e->FirstAttribute(); a; a = a->Next()) {
                BuiltAttr ar{};
                auto name = a->Name();
                auto value = a->Value();
                ar.name_off = sp.add(a->Name());
                ar.value_off = sp.add(a->Value());
                attrs.push_back(ar);
                ++countAttr;
            }
            nodes[myIdx].n.first_attr_idx = firstAttr;
            nodes[myIdx].n.attribute_count = (int16_t)countAttr;

            // children
            int32_t firstChild = (int32_t)nodes.size() - 1;
            int countChild = 0;
            for (auto* c = e->FirstChildElement(); c; c = c->NextSiblingElement()) {
                int idx = buildNode(c, myIdx);
                ++countChild;
            }
            child_table.push_back({ parentId, myIdx });

            nodes[myIdx].n.child_count = (int16_t)countChild;

            // text
            if (e->NoChildren()) {
                if (const char* txt = e->GetText()) {
                    if (*txt) nodes[myIdx].n.content_offset = sp.add(txt);
                }
            }

            return myIdx;
            };

        int32_t rootIdx = buildNode(root, -1);

        for (size_t i = 1; i < nodes.size(); i++)
        {
            nodes[i].n.first_child_idx = nodes[i - 1].n.first_child_idx + nodes[i - 1].n.child_count;
        }

        // --- Write out ---
        std::vector<uint8_t> out;
        out.clear();
        const char magic[] = "CryXmlB";
        out.insert(out.end(), magic, magic + 7);
        out.push_back(0); // pad so u32s align

        auto put32 = [&](uint32_t v) {
            out.push_back(v & 0xFF);
            out.push_back((v >> 8) & 0xFF);
            out.push_back((v >> 16) & 0xFF);
            out.push_back((v >> 24) & 0xFF);
            };

        auto put16 = [&](uint16_t v) {
            out.push_back(v & 0xFF);
            out.push_back((v >> 8) & 0xFF);
            };

        // Reserve space for header (9 * 4 bytes)
        size_t headerPos = out.size();
        printf("headerPos %d\n", (int)headerPos);
        for (int i = 0; i < 9; i++) put32(0);

        // Align to here: node table
        uint32_t node_table_offset = (uint32_t)out.size();
        uint32_t child_table_count = 0;
        for (auto& bn : nodes) {
            auto& n = bn.n;
            put32(n.name_offset);     // relative to strings_offset
            put32(n.content_offset);  // 8 if none
            put16(n.attribute_count);
            put16(n.child_count);
            child_table_count += n.child_count;
            put32(n.parent_id);
            put32(n.first_attr_idx);
            put32(n.first_child_idx);
            put32(n.reserved);
        }

        // Child table
        uint32_t child_table_offset = (uint32_t)out.size();
        std::sort(child_table.begin(), child_table.end(), [](std::tuple<uint32_t, uint32_t> const& t1, std::tuple<uint32_t, uint32_t> const& t2) {
            if (get<0>(t1) == get<0>(t2)) {
                return get<1>(t1) < get<1>(t2);
            }
            return get<0>(t1) < get<0>(t2);
            });

        for (auto idx : child_table) {
            printf("%d %d\n", get<0>(idx), get<1>(idx));
            if (get<0>(idx) != -1) {
                put32(get<1>(idx));
            }
        }

        // Attr table
        uint32_t attr_table_offset = (uint32_t)out.size();
        for (auto& a : attrs) {
            put32(a.name_off);
            put32(a.value_off);
        }

        // String blob
        uint32_t data_table_offset = (uint32_t)out.size();
        out.insert(out.end(), sp.blob.begin(), sp.blob.end());
        uint32_t data_table_size = (uint32_t)(out.size() - data_table_offset);

        // Now patch header
        uint32_t file_size = (uint32_t)out.size();
        uint32_t header_size = 44;
        uint32_t node_table_count = (uint32_t)nodes.size();
        uint32_t attr_table_count = (uint32_t)attrs.size();

        printf("file_size %d\n", file_size);
        printf("node_table_offset %d\n", node_table_offset);
        printf("node_table_count %d\n", node_table_count);
        printf("attr_table_offset %d\n", attr_table_offset);
        printf("attr_table_count %d\n", attr_table_count);


        printf("child_table_offset %d\n", child_table_offset);
        printf("child_table_count %d\n", child_table_count);


        printf("data_table_offset %d\n", data_table_offset);
        printf("data_table_size %d\n", data_table_size);


        auto patch32 = [&](int idx, uint32_t v) {
            size_t pos = headerPos + idx * 4;
            out[pos + 0] = (v & 0xFF);
            out[pos + 1] = ((v >> 8) & 0xFF);
            out[pos + 2] = ((v >> 16) & 0xFF);
            out[pos + 3] = ((v >> 24) & 0xFF);
            };

        patch32(0, file_size);
        patch32(1, node_table_offset);
        patch32(2, node_table_count);
        patch32(3, attr_table_offset);
        patch32(4, attr_table_count);

        patch32(5, child_table_offset);
        patch32(6, child_table_count);

        patch32(7, data_table_offset);
        patch32(8, data_table_size);

        return SaveToFile(output, out);
    }

}