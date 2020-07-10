#include <sstream>
#include <string>
#include <unordered_map>

using namespace std;

#include "ini.h"

namespace Ini {
    Section &Document::AddSection(string name) {
        return sections[move(name)];
    }

    const Section &Document::GetSection(const string &name) const {
        return sections.at(name);
    }

    size_t Document::SectionCount() const {
        return sections.size();
    }


    Document Load(istream &input) {
        Document res;
        Section *current_section = nullptr;

        string line;
        while (getline(input, line)) {
            if (line.empty()) { continue; }
            if (line[0] == '[') {
                size_t bracket_pos = line.rfind(']');
                current_section = &res.AddSection(line.substr(1, bracket_pos - 1));
            } else {
                size_t equal_pos = line.find('=');
                current_section->insert({line.substr(0, equal_pos), line.substr(equal_pos + 1)});
            }
        }
        return res;
    }
}