#include <parser.h>
#include <parser_defs.h>
Rule(Rule) {
    auto pos = in;
    ISC_STD::skip_spaces(pos);
    auto id_res = id(pos);
    if (!id_res.result) {
        printf("Exit 1\n");
        return {};
    }
    pos += id_res.token.length();
    ISC_STD::skip_spaces(pos);
    if (*pos != ':')
        return {};
    pos++;
    ISC_STD::skip_spaces(pos);
    std::vector<::Parser::Rule> rule_results {};
    Rule_result Rule_rule_res;
    printf("enter Rule_rule\n");
    ::Parser::Rule_result data_block_res;
    while ((Rule_rule_res = Rule_rule(pos)).result) {
        rule_results.push_back(Rule_rule_res.token);

        pos += Rule_rule_res.token.length();
        printf("Rule_rule Length: %ld, stopped at: %ld, pos: %c, pos: %ld\n", 
            Rule_rule_res.token.length(),
            pos - in,
            *pos,
            getCurrentPos(pos)
        );
        ISC_STD::skip_spaces(pos);
        data_block_res = Rule_data_block(pos);
        if (data_block_res.result) {
            pos += data_block_res.token.length();
            printf("Matched data block\n");
            break;
        }
    }
    printf("Enter Rule_nested_rule\n");

    std::vector<::Parser::Rule> nested_rule_results {};
    ::Parser::Rule_result Rule_nested_rule_res;
    while (true) {
        Rule_nested_rule_res = Rule_nested_rule(pos);
        if (!Rule_nested_rule_res.result) {
            break;
        }
        nested_rule_results.push_back(Rule_nested_rule_res.token);
        printf("Nested rule length: %zu\n", Rule_nested_rule_res.token.length());
        pos += Rule_nested_rule_res.token.length();
        ISC_STD::skip_spaces(pos);
    }
    printf("Leave rule_nested_rule, total length: %ld\n", pos - in);
    ISC_STD::skip_spaces(pos);
    auto strict_end_result = strict_end(pos);
    if (strict_end_result.result)
        pos += strict_end_result.token.length();
    auto id_res_data = TO(std::string, id_res.token.data);
    const char* kind = (id_res_data[0] >= 'A' && id_res_data[0] <= 'Z') ? "token" : "rule";
    std::unordered_map<const char*, std::any> data {
        { "name", id_res.token },
        { "kind", kind },
        { "rule", rule_results },
        { "data_block", data_block_res.token },
        { "nestedRules", nested_rule_results }
    };
    printf("leave Rule\n");
    RULE_SUCCESSD(in, pos, Rule, data);
}