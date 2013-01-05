#include <iostream>
#include <string>
#include <list>
#include <sstream>
#include <boost/foreach.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

static const int TYPE_INTEGER = 1;
static const int TYPE_STRING = 2;

/*
 * SQL AST structures
 */

/**
 * Field definition.
 */
struct stmt_field
{
    /* Field name */
    std::string name;
    /* Type ID */
    unsigned type;
};

BOOST_FUSION_ADAPT_STRUCT(
    stmt_field,
    (std::string, name)
    (unsigned, type)
)

/*
 * Create table statement
 */
struct stmt_create_table
{
    /*
     * Name of the table
     */
    std::string tablename;
    /*
     * Fields list
     */
    std::list<stmt_field> fields;
};

BOOST_FUSION_ADAPT_STRUCT(
    stmt_create_table,
    (std::string, tablename)
    (std::list<stmt_field>, fields)
)

namespace client
{

using qi::phrase_parse;
using qi::lexeme;
using qi::lit;
using ascii::char_;
using ascii::string;
using ascii::space;

/**
 * Additional keywords
 */
struct data_type_ : qi::symbols<char, unsigned>
{
    data_type_()
    {
        add
            ("integer" , TYPE_INTEGER)
            ("string"  , TYPE_STRING)
        ;
    }
} data_type;

/**
 * SQL grammar definition
 */
template <typename Iterator>
struct sql_grammar : qi::grammar<Iterator, stmt_create_table(), ascii::space_type>
{
    sql_grammar() : sql_grammar::base_type(create_table)
    {
        quoted_string %= lexeme['"' >> +(char_ - '"') >> '"'];
        create_table %=
            lit("CREATE TABLE ")
            >> quoted_string
            >> '('
            >> ((quoted_string >> data_type) % ',')
            >> ')';
    }

    qi::rule<Iterator, std::string(), ascii::space_type> quoted_string;
    qi::rule<Iterator, stmt_create_table(), ascii::space_type> create_table;
};

/* Run parser with grammar*/
stmt_create_table parse_input(std::string const & input)
{
    sql_grammar<std::string::const_iterator> grammar;
    stmt_create_table result;
    std::string::const_iterator first = input.begin(), last = input.end();
    bool r = phrase_parse(
        first,
        last,
        grammar,
        space,
        result
    );
    if (first != last)
        return stmt_create_table();
    return result;
}

}

/* Code generator */
void parse(std::string const & input, std::stringstream & output)
{
    stmt_create_table stmt = client::parse_input(input);
    output << "struct " << stmt.tablename << "_record" << std::endl
           << "{" << std::endl
           << "\t// definition of \"" << stmt.tablename << "\" record." << std::endl;
    BOOST_FOREACH(stmt_field const & field, stmt.fields)
    {
        assert((field.type == TYPE_INTEGER ||
               field.type == TYPE_STRING) && "Unknown SQL type.");
        output << "\t";
        if (field.type == TYPE_INTEGER)
            output << "int";
        else if (field.type == TYPE_STRING)
            output << "std::string";
        output << " " << field.name << ";" << std::endl;
    }
    output << "};" << std::endl;
}

int
main(int argc, char* argv[])
{
    using namespace std;
    std::stringstream ss;
    parse("CREATE TABLE \"asdf\" (\"id\" integer)", ss);
    parse("CREATE TABLE \"asdf\" (\"id\" integer, \"field1\" string)", ss);
    cout << ss.str() << endl;
}