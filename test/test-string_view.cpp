#include "test_app.h"
#include <string_view_.h>

using namespace sys;

class TestStringView : public TestApp
{
public:

    static constexpr const char* src_str = "MOOP";
    static constexpr auto src_str_len = string_view::traits_t::length(src_str);
    static_assert(src_str_len > 0);

    void CheckFundamental()
    {
        stout()->out("Checking fundamentals...\n");

        // Default constructor - empty string
        constexpr string_view sv0;
        static_assert(sv0.data() == nullptr);
        static_assert(sv0.is_empty());
        static_assert(sv0.length() == 0);
        static_assert(!sv0);

        // Construct from a NULL-terminated string
        constexpr string_view sv1(src_str);
        static_assert(sv1.data() == src_str);
        static_assert(!sv1.is_empty());
        static_assert(sv1.length() == src_str_len);
        static_assert(sv1);

        // Construct a piece of a string
        constexpr size_t sv2_len = 2;
        constexpr string_view sv2(src_str, sv2_len);
        static_assert(sv2.data() != nullptr);
        static_assert(!sv2.is_empty());
        static_assert(sv2.length() == sv2_len);
        static_assert(sv2);

        // Copy construct
        constexpr string_view sv3(sv1);
        static_assert(sv3.data() != nullptr);
        static_assert(!sv3.is_empty());
        static_assert(sv3.length() == sv1.length());
        static_assert(sv3);

        // Copy assignment
        constexpr string_view sv4 = sv3;
        static_assert(sv4.data() != nullptr);
        static_assert(!sv4.is_empty());
        static_assert(sv4.length() == sv3.length());
        static_assert(sv4);
    }

    void CheckIterators()
    {
        stout()->out("Checking iterators...\n");

        // Make sure cbegin() == cend() for an empty (nullptr) view
        constexpr string_view sv0;
        static_assert(sv0.cbegin() == sv0.cend());

        // Walk string view and confirm iterator follows raw string data
        constexpr string_view sv1(src_str);
        static_assert(sv1.cbegin() != sv1.cend());
        static_assert(sv1[0] == src_str[0]);
        static_assert(sv1[src_str_len - 1] == src_str[src_str_len - 1]);

        string_view sv2(src_str);
        Verify(&*sv2.cbegin() == &*sv2.begin());
        Verify(&*sv2.cend()   == &*sv2.end());

        // MDTODO : Add reverse iterators when we have them
    }

    void CheckElementAccess()
    {
        stout()->out("Checking element access...\n");

        constexpr string_view sv1(src_str);

        // Check [] and at
        static_assert(sv1[0] == src_str[0]);
        static_assert(&sv1[0] == &const_cast<string_view&>(sv1)[0]);
        static_assert(sv1[src_str_len - 1] == src_str[src_str_len - 1]);
        static_assert(sv1.at(0) == src_str[0]);
        static_assert(sv1.at(src_str_len - 1) == src_str[src_str_len - 1]);

        // Walk all indices
        for (size_t i = 0; i<src_str_len; ++i) {
            if (!Verify(sv1[i]    == src_str[i], "[]: value doesn't match raw data"))
                break;
            if (!Verify(sv1.at(i) == src_str[i], "at: value doesn't match raw data"))
                break;
        }

        // Confirm that at throws on bad index
        bool got_exception = false;
        try { sv1.at(src_str_len); }
        catch (sys::error_bounds&) {
            got_exception = true;
        }
        Verify(got_exception, "Didn't get an exception for bad at() index");

        // Confirm that front and back reference first and last chars, respectively
        constexpr const char* fb_src = "abcdefghijklmnopqrstuvwxyz";
        constexpr auto fb_len = string_view::traits_t::length(fb_src);
        static_assert(&string_view(fb_src).front() == fb_src);
        static_assert(&string_view(fb_src).back()  == fb_src + fb_len - 1);
    }

    void CheckComparison()
    {
        stout()->out("Checking comparisons...\n");

        constexpr const char* src1 = "Moopy";
        constexpr const char* src2 = "Toopy";
        static_assert(src1[0] < src2[0]);

        constexpr const string_view sv1(src1);
        constexpr const string_view sv2(src2);
        static_assert(sv2.compare(src2) == 0);
        static_assert(sv2.compare(src1)  > 0);
        static_assert(sv1.compare(src2)  < 0);

        static_assert(string_view("Loopy").compare("Loopy0") < 0);
        static_assert(string_view("Loopy0").compare("Loopy") > 0);

        static_assert(is_eq (sv1 <=> sv1));
        static_assert(is_neq(sv1 <=> sv2));
        static_assert(is_lt (sv1 <=> sv2));
        static_assert(is_lte(sv1 <=> sv2));
        static_assert(is_lte(sv1 <=> sv1));
        static_assert(is_gt (sv2 <=> sv1));
        static_assert(is_gte(sv2 <=> sv1));
        static_assert(is_gte(sv2 <=> sv2));

        static_assert(sv1 != sv2);
        static_assert(sv1 == string_view(sv1));
    }

    void CheckSubStrings()
    {
        stout()->out("Checking substrings...\n");

        //                            0123456789012
        constexpr const char* src1 = "zzzzzAAAAzzzz";
        constexpr const char* src2 = "AAAAzzzz";
        constexpr const char* src3 = "zzzzzzzAAAA";
        constexpr const char* sub  = "AAAA";
        constexpr auto sub_len = string_view::traits_t::length(sub);

        constexpr string_view sv1(src1);
        constexpr string_view sv2(src2);
        constexpr string_view sv3(src3);
        static_assert(0 == string_view::traits_t::compare(sv1.substr_view(5, 4).data(), sub, sub_len));
        static_assert(0 == string_view::traits_t::compare(sv2.substr_view(0, 4).data(), sub, sub_len));
        static_assert(0 == string_view::traits_t::compare(sv3.substr_view(7, 4).data(), sub, sub_len));
    }

    void CheckSearch()
    {
        stout()->out("Checking searches...\n");

        constexpr auto npos = string_view::npos;

        // starts_with / ends_with
        constexpr const char* src1 = "Pink Floyd";
        static_assert( string_view(src1).starts_with("Pink"));
        static_assert(!string_view(src1).starts_with("Floyd"));
        static_assert( string_view(src1).ends_with("Floyd"));
        static_assert(!string_view(src1).ends_with("Pink"));

        //                                     10        20        30        40        50        60
        //                            0123456789012345678901234567890123456789012345678901234567890123456789
        constexpr const char* src2 = "Ticking away the moments that make up a dull day";
        constexpr const char* src3 = "The time is gone, the song is over, thought I'd something more to say";

        // contains, find_first, find_last, find_first_of
        static_assert( string_view(src2).contains("moments"));
        static_assert(!string_view(src2).contains("brick"));
        static_assert(9    == string_view(src3).find_first("is", 0));
        static_assert(27   == string_view(src3).find_first("is", 10));
        static_assert(npos == string_view(src3).find_first("is", 28));
        static_assert(27   == string_view(src3).find_last("is"));
        static_assert(9    == string_view(src3).find_last("is", 26));
        static_assert(npos == string_view(src3).find_last("is", 8));
        static_assert(45   == string_view(src2).find_last("day"));
        static_assert(npos == string_view(src3).find_first("brick"));
        static_assert(npos == string_view(src3).find_last("brick"));
        // Check other overrides
        static_assert(17   == string_view(src2).find_first("momentary lapse of reason", 0, 6));
        static_assert(18   == string_view(src3).find_last("the song remains the same", npos, 8));
        static_assert(9    == string_view(src3).find_first(string_view("is")));
        static_assert(40   == string_view(src2).find_last(string_view("dull")));
        static_assert(12   == string_view(src3).find_first('g'));
        static_assert(0    == string_view(src3).find_last('T'));
        static_assert(68   == string_view(src3).find_last('y'));
        // Empty strings and searches
        static_assert(0    == string_view(src3).find_first(""));        // Empty search
        static_assert(npos == string_view().find_first("anything"));    // Empty source
        static_assert(npos == string_view().find_first(string_view())); // Both empty
        static_assert(npos != string_view(src3).find_last(""));  // empty search
        static_assert(npos == string_view().find_last("anything"));
        static_assert(npos == string_view().find_last(string_view()));

        // find_first_of
        //                            01234567890123456789012345
        constexpr const char* src4 = "abcdefghijklmnopqrstuvwxyz";
        constexpr const char* vowels = "aeiou"; // You can pound sand, y.
        constexpr const char* consonants = "bcdfghjklmnpqrstvwxyz";
        constexpr string_view ff(src4);
        constexpr string_view svv(vowels);
        static_assert(0    == ff.find_first_of(svv, 0));
        static_assert(4    == ff.find_first_of(svv, 1));
        static_assert(8    == ff.find_first_of(svv, 5));
        static_assert(14   == ff.find_first_of(svv, 9));
        static_assert(20   == ff.find_first_of(svv, 15));
        static_assert(npos == ff.find_first_of(svv, 21));
        // overloads
        static_assert(12   == ff.find_first_of('m'));
        static_assert(npos == ff.find_first_of('m', 13));
        static_assert(14   == ff.find_first_of(vowels, 9));
        static_assert(npos == ff.find_first_of("000abc", 3, 0));
        // Empty strings and searches
        static_assert(npos == string_view().find_first_of("anything"));         // Empty source
        static_assert(npos == ff.find_first_of(string_view()));                 // Empty search set
        static_assert(npos == string_view().find_first_of(string_view()));      // Both empty
        // find_first_not_of
        constexpr string_view svc(consonants);
        static_assert(0    == ff.find_first_not_of(svc, 0));
        static_assert(4    == ff.find_first_not_of(svc, 1));
        static_assert(8    == ff.find_first_not_of(svc, 5));
        static_assert(14   == ff.find_first_not_of(svc, 9));
        static_assert(20   == ff.find_first_not_of(svc, 15));
        static_assert(npos == ff.find_first_not_of(svc, 21));
        // overloads
        static_assert(4    == string_view("0000abcdef").find_first_not_of('0'));
        static_assert(npos == string_view("0000000000").find_first_not_of('0'));
        static_assert(0    == ff.find_first_not_of(consonants, 0));
        static_assert(npos == string_view("0000").find_first_not_of("abc0", 3));
        // Empty strings and searches
        static_assert(npos == string_view().find_first_not_of("anything"));     // Empty source
        static_assert(npos != ff.find_first_not_of(string_view()));             // Empty search set
        static_assert(npos == string_view().find_first_not_of(string_view()));  // Both empty

        // find_last_of
        //                                     10
        constexpr const char* src5 = "01234567890123456789";
        constexpr string_view fl(src5);
        static_assert(19   == fl.find_last_of(string_view("987")));
        static_assert( 9   == fl.find_last_of(string_view("987"), 16));
        // overloads
        static_assert(19   == fl.find_last_of("987"));
        static_assert(16   == fl.find_last_of("456789", 3, npos));
        static_assert(17   == fl.find_last_of('7'));
        static_assert( 7   == fl.find_last_of('7', 16));
        // Empty string and searches
        static_assert(npos == string_view().find_last_of("anything"));      // Empty source
        static_assert(npos == fl.find_last_of(string_view()));              // Empty search set
        static_assert(npos == string_view().find_last_of(string_view()));   // Both empty
        // find_last_not_of
        static_assert(16   == fl.find_last_not_of(string_view("987")));
        static_assert( 6   == fl.find_last_not_of(string_view("987"), 9));
        // overloads
        static_assert(18   == fl.find_last_not_of('9'));
        static_assert(16   == fl.find_last_not_of("987"));
        static_assert(16   == fl.find_last_not_of("987654", 3, npos));
        // Empty string and seaches
        static_assert(npos == string_view().find_last_not_of("anything"));      // Empty source
        static_assert(19   == fl.find_last_not_of(string_view()));              // Empty search set
        static_assert(npos == string_view().find_last_not_of(string_view()));   // Both empty
    }

    void CheckModifiers()
    {
        stout()->out("Checking modifiers...\n");

        static_assert(0 == string_view("MOOPstring").remove_prefix(4).compare("string"));
        static_assert(0 == string_view("stringMOOP").remove_suffix(4).compare("string"));

        static_assert(0 == string_view("   Thinner!   ").trim(true,  true ).compare("Thinner!"));
        static_assert(0 == string_view("   Thinner!   ").trim(false, false).compare("   Thinner!   "));
        static_assert(0 == string_view("   Thinner!   ").trim(true,  false).compare("Thinner!   "));
        static_assert(0 == string_view("   Thinner!   ").trim(false, true ).compare("   Thinner!"));
        static_assert(string_view("   \t\n\r\f\v").trim().is_empty());
        static_assert(string_view("   \t\n\r\f\v").trim(false, true).is_empty());
        static_assert(string_view("   \t\n\r\f\v").trim(true, false).is_empty());
        static_assert(string_view("").trim().is_empty());

        string_view sv1("string one");
        string_view sv1c(sv1);
        string_view sv2("string two");
        string_view sv2c(sv2);

        sv1.swap(sv2);

        Verify(sv1 == sv2c);
        Verify(sv2 == sv1c);
    }

    bool RunTests() override
    {
        CheckFundamental();
        CheckIterators();
        CheckElementAccess();
        CheckComparison();
        CheckSubStrings();
        CheckSearch();
        CheckModifiers();

        return true;
    }
};

sys::app* CreateApp()
{
    return new TestStringView();
}
