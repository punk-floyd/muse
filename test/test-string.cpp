#include "test_app.h"
#include <string_.h>

using namespace sys;

class TestCharConv : public TestApp
{
public:

    void CheckFundamental()
    {
        stout()->out("Checking fundamentals...\n");

        // Default constructor - empty string
        constexpr sys::string s0;
        static_assert(s0.data() != nullptr);        // Always have \0, so never null
        static_assert(s0.length() == 0);
        static_assert(s0.is_empty());

        // Construct from a NULL-terminated string (short)
        constexpr const char* src1 = "Not empty";
        constexpr size_t src1_len = string::traits_t::length(src1);
        static_assert(src1_len && (src1_len <= string::sso_capacity()));
        static_assert(string(src1).data()   != nullptr);
        static_assert(string(src1).length() == src1_len);
        static_assert(!string(src1).is_empty());

        // Construct from a NULL-terminater string (long)
        constexpr const char* src2 = "Not empty and not very short either";
        constexpr size_t src2_len = string::traits_t::length(src2);
        static_assert(src2_len && (src2_len > string::sso_capacity()));
        static_assert(string(src2).data()   != nullptr);
        static_assert(string(src2).length() == src2_len);
        static_assert(!string(src2).is_empty());

        // Construct from a piece of a string
        constexpr const char* src3 = "Moopy Troopy";
        static_assert(string(src3, 5).data() != nullptr);
        static_assert(string(src3, 5).length() == 5);
        static_assert(!string(src3, 5).is_empty());

        // Construct as a character fill
        static_assert(string('A', 10).data() != nullptr);
        static_assert(string('A', 10).length() == 10);
        static_assert(!string('A', 10).is_empty());

        // Construct from a string_view
        constexpr string_view sv1("Your face");
        constexpr auto sv1_len = sv1.length();
        static_assert(string(sv1).data() != nullptr);
        static_assert(string(sv1).length() == sv1_len);
        static_assert(!string(sv1).is_empty());

        // Construct from a piece of a string_view
        static_assert(string(sv1, 5).data() != nullptr);
        static_assert(string(sv1, 5).length() == 4);
        static_assert(!string(sv1, 5).is_empty());

        {
            // Copy constructor (short)
            constexpr const char* my_src = "source string";
            static_assert(string::traits_t::length(my_src) <= string::sso_capacity());
            string s1(my_src);      // Source string
            string s2(s1);          // Test string
            VerifyThrow(0 == sys::string::traits_t::compare(s1.data(), s2.data()));
        }

        {
            // Copy constructor (long)
            constexpr const char* my_src = "source string that is longer";
            static_assert(string::traits_t::length(my_src) > string::sso_capacity());
            string s1(my_src);      // Source string
            string s2(s1);          // Test string
            VerifyThrow(0 == sys::string::traits_t::compare(s1.data(), s2.data()));
        }

        {
            // Move constructor (short)
            constexpr const char* my_src = "source string";
            static_assert(string::traits_t::length(my_src) <= string::sso_capacity());
            string s1(my_src);
            string s2(sys::move(s1));
            VerifyThrow(0 == sys::string::traits_t::compare(s2.data(), my_src));
            // Short mode strings are unaffected when moved from
            VerifyThrow(!s1.is_empty());
        }

        {
            // Move constructor (long)
            constexpr const char* my_src = "source string this is longer than your mom";
            static_assert(string::traits_t::length(my_src) > string::sso_capacity());
            string s1(my_src);
            string s2(sys::move(s1));
            VerifyThrow(0 == sys::string::traits_t::compare(s2.data(), my_src));
            // Long mode strings are invalidated when moved from
            VerifyThrow(s1.is_empty());
        }

        {
            // Copy assignment (short)
            constexpr const char* my_src = "source string";
            static_assert(string::traits_t::length(my_src) <= string::sso_capacity());
            string s1(my_src);      // Source string
            string s2;              // Test string
            s2 = s1;
            VerifyThrow(0 == sys::string::traits_t::compare(s1.data(), s2.data()));
            VerifyThrow(s1.capacity() == s2.capacity());
            VerifyThrow(s1.length()   == s2.length());
        }

        {
            // Copy assignment (long)
            constexpr const char* my_src = "source string that is longer";
            static_assert(string::traits_t::length(my_src) > string::sso_capacity());
            string s1(my_src);      // Source string
            string s2;              // Test string
            s2 = s1;
            VerifyThrow(0 == sys::string::traits_t::compare(s1.data(), s2.data()));
            VerifyThrow(s1.capacity() == s2.capacity());
            VerifyThrow(s1.length()   == s2.length());
        }

        {
            // Move assignment (short)
            constexpr const char* my_src = "source string";
            static_assert(string::traits_t::length(my_src) <= string::sso_capacity());
            string s1(my_src);
            string s2;
            s2 = sys::move(s1);
            VerifyThrow(0 == sys::string::traits_t::compare(s2.data(), my_src));
            // Short mode strings are unaffected when moved from
            VerifyThrow(!s1.is_empty());
        }

        {
            // Move assignment (long)
            constexpr const char* my_src = "source string this is longer than your mom";
            static_assert(string::traits_t::length(my_src) > string::sso_capacity());
            string s1(my_src);
            string s2;
            s2 = sys::move(s1);
            VerifyThrow(0 == sys::string::traits_t::compare(s2.data(), my_src));
            // Long mode strings are invalidated when moved from
            VerifyThrow(s1.is_empty());
        }

        {
            // Assignment from a NULL-terminated string (short)
            constexpr const char* my_src = "source string";
            static_assert(string::traits_t::length(my_src) <= string::sso_capacity());
            string s1;
            s1 = my_src;
            VerifyThrow(0 == sys::string::traits_t::compare(s1.data(), my_src));
        }

        {
            // Assignment from a NULL-terminated string (long)
            constexpr const char* my_src = "source string that is longer";
            static_assert(string::traits_t::length(my_src) > string::sso_capacity());
            string s2;              // Test string
            s2 = my_src;
            VerifyThrow(0 == sys::string::traits_t::compare(s2.data(), my_src));
        }

        {
            // Assignment from something string-view-like
            constexpr string_view sv_src = "salsa shark!";
            string s;
            s = sv_src;
            VerifyThrow(0 == sys::string::traits_t::compare(s.data(), sv_src.data()));
        }

        {
            // operator string_view
            string s("seas of cheese");
            VerifyThrow(!static_cast<string_view>(s).is_empty());
        }
    }

    void CheckAssign()
    {
        stout()->out("Checking assignment...\n");

        static_assert(0 == string().assign('X', 3).compare("XXX"));
        static_assert(0 == string().assign(string("tuba")).compare("tuba"));
        static_assert(0 == string().assign(string("tuna fish salad"), 0, 4).compare("tuna"));

        static_assert(0 == string().assign("Rocky III", 5).compare("Rocky"));
        static_assert(0 == string().assign("Poop").compare("Poop"));
        static_assert(0 == string().assign(string_view("Dilbert")).compare("Dilbert"));
        static_assert(0 == string().assign(string_view("Superman IV"), 9, 1).compare("I"));

        // Make sure the old content is removed
        static_assert(0 == string("non-empty").assign("Your mom").compare("Your mom"));

        {
            // Move assignment (short)
            constexpr const char* my_src = "Flippy";
            static_assert(string::traits_t::length(my_src) <= string::sso_capacity());
            string movee(my_src);
            string s;
            s.assign(sys::move(movee));
            Verify(0 == s.compare(my_src));
            // Short mode strings are unaffected when moved from
            Verify(!movee.is_empty());
        }

        {
            // Move assignment (long)
            constexpr const char* my_src = "Flippy dippy sippy";
            static_assert(string::traits_t::length(my_src) > string::sso_capacity());
            string movee(my_src);
            string s;
            s.assign(sys::move(movee));
            Verify(0 == s.compare(my_src));
            // Long mode strings are invalidated when moved from
            Verify(movee.is_empty());
        }

    }

    void CheckIterators()
    {
        stout()->out("Checking iterators...\n");

        constexpr const char* src_str = "bold and disastrous";
        string s(src_str);

        // cbegin/cend
        for (size_t idx = 0; const auto it : s)
            Verify(it == src_str[idx++]);
        // begin/end
        for (size_t idx = 0; auto it : s)
            Verify(it == src_str[idx++]);

        // MDTODO : Add reverse iterators when we have them
    }

    void CheckElementAccess()
    {
        stout()->out("Checking element access...\n");

        constexpr const char* my_src = "Drop it, Buster";
        constexpr size_t src_len = string::traits_t::length(my_src);

        static_assert(string(my_src)[0]         == my_src[0]);
        static_assert(string(my_src)[src_len-1] == my_src[src_len-1]);
        static_assert(string(my_src).at(0)         == my_src[0]);
        static_assert(string(my_src).at(src_len-1) == my_src[src_len-1]);

        string s(my_src);

        // Walk all indices
        for (size_t i = 0; i<src_len; ++i) {
            if (!Verify(s[i]    == my_src[i], "[]: value doesn't match raw data"))
                break;
            if (!Verify(s.at(i) == my_src[i], "at: value doesn't match raw data"))
                break;
        }

        // Confirm that at throws on bad index
        bool got_exception = false;
        try { s.at(src_len); }
        catch (sys::error_bounds&) {
            got_exception = true;
        }
        Verify(got_exception, "Didn't get an exception for bad at() index");

        // Confirm that front and back reference first and last chars, respectively
        constexpr const char* fb_src = "abcdefghijklmnopqrstuvwxyz";
        constexpr auto fb_len = string::traits_t::length(fb_src);
        static_assert(string(fb_src).front() == fb_src[0]);
        static_assert(string(fb_src).back()  == fb_src[fb_len - 1]);
    }

    void CheckComparison()
    {
        stout()->out("Checking comparisons...\n");

        constexpr const char* src1 = "Moopy";
        constexpr const char* src2 = "Toopy";
        static_assert(src1[0] < src2[0]);

        // Note: All string::compare routines are implemented by
        // string_view::compare

        // compare (1)
        static_assert(string(src1).compare(string(src1)) == 0);
        static_assert(string(src2).compare(string(src1))  > 0);
        static_assert(string(src1).compare(string(src2))  < 0);
        // Try all the variants of compare....
        static_assert(string("xxxAAAAxxxx").compare(3, 4, string("AAAA")) == 0);    // (2)
        static_assert(string("xxxAAAAxxxx").compare(3, 4, string("zzzzzzAAAAzzzz"), 6, 4) == 0); // (3)
        static_assert(string("MOOPY").compare("MOOPY") == 0);   // (4)
        static_assert(string("xxxxMOOPYxxxx").compare(4, 5, "MOOPY") == 0); // (5)
        static_assert(string("Floopy").compare(string_view("Floopy")) == 0);    // (6)
        static_assert(string("zzScoopyzzzzz").compare(2, 6, string_view("Scoopy")) == 0); // (7)
        static_assert(string("zzScoopyzzzzz").compare(2, 6, string_view("xxxxScoopyxxx"), 4, 6) == 0); // (8)

        static_assert(is_eq (string(src1) <=> string(src1)));
        static_assert(is_neq(string(src1) <=> string(src2)));
        static_assert(is_lt (string(src1) <=> string(src2)));
        static_assert(is_lte(string(src1) <=> string(src2)));
        static_assert(is_lte(string(src1) <=> string(src1)));
        static_assert(is_gt (string(src2) <=> string(src1)));
        static_assert(is_gte(string(src2) <=> string(src1)));
        static_assert(is_gte(string(src2) <=> string(src2)));

        static_assert(string(src1) == string(src1));
        static_assert(string(src1) != string(src2));
    }

    void CheckSubStrings()
    {
        stout()->out("Checking substrings...\n");

        //                            0123456789012
        constexpr const char* src1 = "zzzzzAAAAzzzz";
        constexpr const char* src2 = "AAAAzzzz";
        constexpr const char* src3 = "zzzzzzzAAAA";
        constexpr const char* sub  = "AAAA";
        constexpr auto sub_len = string::traits_t::length(sub);

        string sv1(src1);
        string sv2(src2);
        string sv3(src3);

        // substr_view -> returns a string_view
        Verify(0 == sys::string::traits_t::compare(sv1.substr_view(5, 4).data(), sub, sub_len));
        Verify(0 == sys::string::traits_t::compare(sv2.substr_view(0, 4).data(), sub, sub_len));
        Verify(0 == sys::string::traits_t::compare(sv3.substr_view(7, 4).data(), sub, sub_len));

        // substr -> returns a new string
        Verify(0 == sys::string::traits_t::compare(sv1.substr(5, 4).data(), sub, sub_len));
        Verify(0 == sys::string::traits_t::compare(sv2.substr(0, 4).data(), sub, sub_len));
        Verify(0 == sys::string::traits_t::compare(sv3.substr(7, 4).data(), sub, sub_len));
    }

    void CheckSearch()
    {
        stout()->out("Checking searches...\n");

        constexpr auto npos = string_view::npos;

        // starts_with / ends_with
        constexpr const char* src1 = "Pink Floyd";
        static_assert( string(src1).starts_with("Pink"));
        static_assert(!string(src1).starts_with("Floyd"));
        static_assert( string(src1).ends_with("Floyd"));
        static_assert(!string(src1).ends_with("Pink"));

        //                                     10        20        30        40        50        60
        //                            0123456789012345678901234567890123456789012345678901234567890123456789
        constexpr const char* src2 = "Ticking away the moments that make up a dull day";
        constexpr const char* src3 = "The time is gone, the song is over, thought I'd something more to say";

        // contains, find_first, find_last, find_first_of
        static_assert( string(src2).contains("moments"));
        static_assert(!string(src2).contains("brick"));
        static_assert(9    == string(src3).find_first("is", 0));
        static_assert(27   == string(src3).find_first("is", 10));
        static_assert(npos == string(src3).find_first("is", 28));
        static_assert(27   == string(src3).find_last("is"));
        static_assert(9    == string(src3).find_last("is", 26));
        static_assert(npos == string(src3).find_last("is", 8));
        static_assert(45   == string(src2).find_last("day"));
        static_assert(npos == string(src3).find_first("brick"));
        static_assert(npos == string(src3).find_last("brick"));
        // Check other overrides
        static_assert(17   == string(src2).find_first("momentary lapse of reason", 0, 6));
        static_assert(18   == string(src3).find_last("the song remains the same", npos, 8));
        static_assert(9    == string(src3).find_first(string_view("is")));
        static_assert(40   == string(src2).find_last(string_view("dull")));
        static_assert(12   == string(src3).find_first('g'));
        static_assert(0    == string(src3).find_last('T'));
        static_assert(68   == string(src3).find_last('y'));
        // Empty strings and searches
        static_assert(0    == string(src3).find_first(""));        // Empty search
        static_assert(npos == string().find_first("anything"));    // Empty source
        static_assert(npos == string().find_first(string_view())); // Both empty
        static_assert(npos != string(src3).find_last(""));  // empty search
        static_assert(npos == string().find_last("anything"));
        static_assert(npos == string().find_last(string_view()));

        // find_first_of
        //                            01234567890123456789012345
        constexpr const char* src4 = "abcdefghijklmnopqrstuvwxyz";
        constexpr const char* vowels = "aeiou"; // You can pound sand, y.
        constexpr const char* consonants = "bcdfghjklmnpqrstvwxyz";
        static_assert(0    == string(src4).find_first_of(string(vowels), 0));
        static_assert(4    == string(src4).find_first_of(string(vowels), 1));
        static_assert(8    == string(src4).find_first_of(string(vowels), 5));
        static_assert(14   == string(src4).find_first_of(string(vowels), 9));
        static_assert(20   == string(src4).find_first_of(string(vowels), 15));
        static_assert(npos == string(src4).find_first_of(string(vowels), 21));
        // overloads
        static_assert(12   == string(src4).find_first_of('m'));
        static_assert(npos == string(src4).find_first_of('m', 13));
        static_assert(14   == string(src4).find_first_of(string(vowels), 9));
        static_assert(npos == string(src4).find_first_of("000abc", 3, 0));
        // Empty strings and searches
        static_assert(npos == string().find_first_of("anything"));          // Empty source
        static_assert(npos == string(src4).find_first_of(string()));        // Empty search set
        static_assert(npos == string().find_first_of(string()));            // Both empty
        // find_first_not_of
        static_assert(0    == string(src4).find_first_not_of(string(consonants), 0));
        static_assert(4    == string(src4).find_first_not_of(string(consonants), 1));
        static_assert(8    == string(src4).find_first_not_of(string(consonants), 5));
        static_assert(14   == string(src4).find_first_not_of(string(consonants), 9));
        static_assert(20   == string(src4).find_first_not_of(string(consonants), 15));
        static_assert(npos == string(src4).find_first_not_of(string(consonants), 21));
        // overloads
        static_assert(4    == string("0000abcdef").find_first_not_of('0'));
        static_assert(npos == string("0000000000").find_first_not_of('0'));
        static_assert(0    == string(src4).find_first_not_of(string(consonants), 0));
        static_assert(npos == string("0000").find_first_not_of("abc0", 3));
        // Empty strings and searches
        static_assert(npos == string().find_first_not_of("anything"));      // Empty source
        static_assert(npos != string(src4).find_first_not_of(string()));    // Empty search set
        static_assert(npos == string().find_first_not_of(string()));        // Both empty

        // find_last_of
        //                                     10
        constexpr const char* src5 = "01234567890123456789";
        static_assert(19   == string(src5).find_last_of(string("987")));
        static_assert( 9   == string(src5).find_last_of(string("987"), 16));
        // overloads
        static_assert(19   == string(src5).find_last_of("987"));
        static_assert(16   == string(src5).find_last_of("456789", 3, npos));
        static_assert(17   == string(src5).find_last_of('7'));
        static_assert( 7   == string(src5).find_last_of('7', 16));
        // Empty string and searches
        static_assert(npos == string().find_last_of("anything"));       // Empty source
        static_assert(npos == string(src5).find_last_of(string()));     // Empty search set
        static_assert(npos == string().find_last_of(string()));         // Both empty
        // find_last_not_of
        static_assert(16   == string(src5).find_last_not_of(string("987")));
        static_assert( 6   == string(src5).find_last_not_of(string("987"), 9));
        // overloads
        static_assert(18   == string(src5).find_last_not_of('9'));
        static_assert(16   == string(src5).find_last_not_of("987"));
        static_assert(16   == string(src5).find_last_not_of("987654", 3, npos));
        // Empty string and seaches
        static_assert(npos == string().find_last_not_of("anything"));       // Empty source
        static_assert(19   == string(src5).find_last_not_of(string()));     // Empty search set
        static_assert(npos == string().find_last_not_of(string()));         // Both empty
    }

    void CheckAppend()
    {
        stout()->out("Checking append...\n");

        static_assert(0 == string("ff").append('i', 3).compare("ffiii"));
        static_assert(0 == string("Mike ").append(string("DeKoker")).compare("Mike DeKoker"));
        static_assert(0 == string("banana").append(string("I found fishbones"), 8, 9).compare("bananafishbones"));
        static_assert(0 == string("chicken").append("pox on your house", 3).compare("chickenpox"));
        static_assert(0 == string("pot").append("pie").compare("potpie"));
        static_assert(0 == string("string").append(string_view("_view")).compare("string_view"));
        static_assert(0 == string("Muppet").append(string_view("That's show business!"), 6, 5).compare("Muppet show"));

        static_assert(0 == string("Star Wars").operator+=(string(": A New Hope")).compare("Star Wars: A New Hope"));
        static_assert(0 == string("Mike ").operator+=('D').compare("Mike D"));
        static_assert(0 == string("Smurf ").operator+=("Soup").compare("Smurf Soup"));
        static_assert(0 == string("Mr ").operator+=(string_view("Twisted Sister")).compare("Mr Twisted Sister"));
    }

    void CheckInsert()
    {
        stout()->out("Checking insert...\n");

        static_assert(0 == string().insert(0, 'A', 3).compare("AAA"));
        static_assert(0 == string("No no glory").insert(2, " guts").compare("No guts no glory"));
        static_assert(0 == string("Michael ").insert(string::npos, "Jackson Browne", 7).compare("Michael Jackson"));
        static_assert(0 == string("Silly").insert(string::npos, string(" Putty")).compare("Silly Putty"));
        static_assert(0 == string("ABEFG").insert(2, string("ABCDEFG"), 2, 2).compare("ABCDEFG"));
        static_assert(0 == string("Spelling B").insert(string::npos, 'e', 2).compare("Spelling Bee"));
        static_assert(0 == string("Pac Man").insert(0, string_view("Ms. ")).compare("Ms. Pac Man"));
        static_assert(0 == string("Steve Mc").insert(string::npos, string_view("Remember Queensryche?"), 9, 5).compare("Steve McQueen"));

        // Cause switch from short to long mode
        static_assert(0 == string("0123456789").insert(string::npos, "01234567890123456789").compare("012345678901234567890123456789"));
    }

    void CheckReplace()
    {
        stout()->out("Checking replace...\n");

        static_assert(0 == string("aaaXXXXaaaaa").replace(3, 4, string("Fonzie")).compare("aaaFonzieaaaaa"));
        // (2) below
        static_assert(0 == string("aaaXXXXaaaaa").replace(3, 4, string("Arty Fonzie III"), 5, 6).compare("aaaFonzieaaaaa"));
        // (4) below
        static_assert(0 == string("aaaXXXXaaaaa").replace(3, 4, "Fonzie III", 6).compare("aaaFonzieaaaaa"));
        // (6) below
        static_assert(0 == string("aaaXXXXaaaaa").replace(3, 4, "Fonzie").compare("aaaFonzieaaaaa"));
        // (8) below
        static_assert(0 == string("aaaXXXXaaaaa").replace(3, 4, 'F', 6).compare("aaaFFFFFFaaaaa"));
        static_assert(0 == string("aaaXXXXaaaaa").replace(3, 4, string_view("Fonzie")).compare("aaaFonzieaaaaa"));
        // (11) below
        static_assert(0 == string("aaaXXXXaaaaa").replace(3, 4, string_view("Arty Fonzie III"), 5, 6).compare("aaaFonzieaaaaa"));

        {   // (2)
            string s0("aaaXXXXaaaaa");
            Verify(0 == s0.replace(s0.cbegin() + 3, s0.cbegin() + 7, string("Fonzie")).compare("aaaFonzieaaaaa"));
        }

        // (4) TODO : Don't have input iterators yet

        {   // (6)
            string s0("aaaXXXXaaaaa");
            Verify(0 == s0.replace(s0.cbegin() + 3, s0.cbegin() + 7, "Fonzie III", 6).compare("aaaFonzieaaaaa"));
        }

        {   // (8)
            string s0("aaaXXXXaaaaa");
            Verify(0 == s0.replace(s0.cbegin() + 3, s0.cbegin() + 7, "Fonzie").compare("aaaFonzieaaaaa"));
        }

        {   // (11)
            string s0("aaaXXXXaaaaa");
            Verify(0 == s0.replace(s0.cbegin() + 3, s0.cbegin() + 7, string_view("Fonzie")).compare("aaaFonzieaaaaa"));
        }
    }

    void CheckTrim()
    {
        stout()->out("Checking trim...\n");

        static_assert(0 == string("   Thinner!   ").trim(true,  true ).compare("Thinner!"));
        static_assert(0 == string("   Thinner!   ").trim(false, false).compare("   Thinner!   "));
        static_assert(0 == string("   Thinner!   ").trim(true,  false).compare("Thinner!   "));
        static_assert(0 == string("   Thinner!   ").trim(false, true ).compare("   Thinner!"));
        static_assert(string("   \t\n\r\f\v").trim().is_empty());
        static_assert(string("   \t\n\r\f\v").trim(false, true).is_empty());
        static_assert(string("   \t\n\r\f\v").trim(true, false).is_empty());
        static_assert(string("").trim().is_empty());
    }

    bool RunTests() override
    {
        CheckFundamental();
        CheckAssign();
        CheckIterators();
        CheckElementAccess();
        CheckComparison();
        CheckSubStrings();
        CheckSearch();
        CheckAppend();
        CheckInsert();
        CheckReplace();
        CheckTrim();

        return true;
    }
};

sys::app* CreateApp()
{
    return new TestCharConv();
}
