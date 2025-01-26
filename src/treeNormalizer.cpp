#include <parser.h>
#include <internal_types.h>
#include <token_management.h>
#include <corelib.h>
void normalizeHelper(arr_t<Parser::Rule> &rules) {
    arr_t<Parser::Rule> ops;
    Parser::Rule prev_rule;
    
    bool in_op = false, prev_op = false; // Initialize flags
    int begin = 0;
    for (int i = 0; i < rules.size(); i++) {
        auto el = rules[i];
        auto rule_data = std::any_cast<obj_t>(el.data);
        auto rule = std::any_cast<Parser::Rule>(corelib::map::get(rule_data, "val"));

        if (rule.name == Parser::Rules::Rule_group) {
            auto rules = rule.as<arr_t<Parser::Rule>>();
            normalizeHelper(rules);
            
        }
        if (rule.name == Parser::Rules::Rule_op) {
            if (!in_op) {
                in_op = true;
                ops.push_back(prev_rule);
                begin = i - 1; // Track the start of the operator sequence
            }
            //ops.push_back(rule); // Add operator to ops
            prev_op = true;
        } else if (prev_op) {
            ops.push_back(rule); // Add non-operator to ops
            prev_op = false;
        } else {
            if (in_op) {
                // Create a new rule with accumulated operators and operands
                auto new_rule = Tokens::make_rule(Parser::Rules::Rule_op, ops);
                obj_t new_rule_data = {
                    { "val", new_rule },
                    { "qualifier", Parser::Rule() }
                };
                auto new_token = Tokens::make_rule(Parser::Rules::Rule_rule, new_rule_data);
                // Replace the operator sequence in `rules` with the new rule
                rules.erase(rules.begin() + begin, rules.begin() + i + 1);
                rules.insert(rules.begin() + begin, new_token);

                // Adjust the loop variables
                i = begin;  // Reset `i` to the start of the inserted rule
                in_op = false;  // Reset the in_op flag
                ops.clear();    // Clear accumulated operators
            }

        }

        prev_rule = rule; // Update the previous rule
    }


    // Handle any remaining operator sequences at the end
    if (in_op && !ops.empty()) {
        auto new_rule = Tokens::make_rule(Parser::Rules::Rule_op, ops);
        obj_t new_rule_data = {
            { "val", new_rule },
            { "qualifier", Parser::Rule() }
        };
        auto new_token = Tokens::make_rule(Parser::Rules::Rule_rule, new_rule_data);
        rules.erase(rules.begin() + begin, rules.end());
        rules.push_back(new_token);
    }
}
void normalizeRule(Parser::Rule &member) {
    auto data = std::any_cast<obj_t>(member.data);
    auto rules = std::any_cast<arr_t<Parser::Rule>>(corelib::map::get(data, "rule"));

    normalizeHelper(rules);

    // Update the modified `rules` back into the member
    corelib::map::set(data, "rule", std::any(rules));
    member.data = data;
}


void normalizeTree(Parser::Tree &tree) {
    for (auto &member : tree) {
        if (member.name == Parser::Rules::Rule) {
            normalizeRule(member);
        }
    }
}