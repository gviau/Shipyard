#include <catch.hpp>

#include <system/string.h>

TEST_CASE("Test StringA", "[String]")
{
    Shipyard::StringA string;

    SECTION("Default Constructor")
    {
        REQUIRE(string.Size() == 0);
        REQUIRE(string.Capacity() >= 1);
        REQUIRE(string.GetBuffer()[0] == '\0');
    }

    SECTION("String Constructor")
    {
        Shipyard::StringA constructedString("Test");

        REQUIRE(constructedString.Size() == 4);
        REQUIRE(constructedString.Capacity() >= 5);
        REQUIRE(constructedString[0] == 'T');
        REQUIRE(constructedString[1] == 'e');
        REQUIRE(constructedString[2] == 's');
        REQUIRE(constructedString[3] == 't');
        REQUIRE(constructedString.GetBuffer()[4] == '\0');
    }

    SECTION("String Constructor Sub")
    {
        Shipyard::StringA constructedString("Test", 3);

        REQUIRE(constructedString.Size() == 3);
        REQUIRE(constructedString.Capacity() >= 4);
        REQUIRE(constructedString[0] == 'T');
        REQUIRE(constructedString[1] == 'e');
        REQUIRE(constructedString[2] == 's');
        REQUIRE(constructedString.GetBuffer()[3] == '\0');
    }

    SECTION("String Copy Constructor")
    {
        Shipyard::StringA copiedString = string;

        REQUIRE(string.Size() == 0);
        REQUIRE(string.Capacity() >= 1);
        REQUIRE(string.GetBuffer()[0] == '\0');

        Shipyard::StringA initializedString = "Test";

        REQUIRE(initializedString.Size() == 4);
        REQUIRE(initializedString.Capacity() >= 5);
        REQUIRE(initializedString[0] == 'T');
        REQUIRE(initializedString[1] == 'e');
        REQUIRE(initializedString[2] == 's');
        REQUIRE(initializedString[3] == 't');
        REQUIRE(initializedString.GetBuffer()[4] == '\0');

        Shipyard::StringA anotherCopiedString = initializedString;

        REQUIRE(anotherCopiedString.Size() == 4);
        REQUIRE(anotherCopiedString.Capacity() >= 5);
        REQUIRE(anotherCopiedString[0] == 'T');
        REQUIRE(anotherCopiedString[1] == 'e');
        REQUIRE(anotherCopiedString[2] == 's');
        REQUIRE(anotherCopiedString[3] == 't');
        REQUIRE(anotherCopiedString.GetBuffer()[4] == '\0');
    }

    SECTION("String assignation")
    {
        Shipyard::StringA anotherString = "Test";

        string = anotherString;

        REQUIRE(string.Size() == 4);
        REQUIRE(string.Capacity() >= 5);
        REQUIRE(string[0] == 'T');
        REQUIRE(string[1] == 'e');
        REQUIRE(string[2] == 's');
        REQUIRE(string[3] == 't');
        REQUIRE(string.GetBuffer()[4] == '\0');

        string = "sub";

        REQUIRE(string.Size() == 3);
        REQUIRE(string.Capacity() >= 4);
        REQUIRE(string[0] == 's');
        REQUIRE(string[1] == 'u');
        REQUIRE(string[2] == 'b');
        REQUIRE(string.GetBuffer()[3] == '\0');

        string = "larger";

        REQUIRE(string.Size() == 6);
        REQUIRE(string.Capacity() >= 7);
        REQUIRE(string[0] == 'l');
        REQUIRE(string[1] == 'a');
        REQUIRE(string[2] == 'r');
        REQUIRE(string[3] == 'g');
        REQUIRE(string[4] == 'e');
        REQUIRE(string[5] == 'r');
        REQUIRE(string.GetBuffer()[6] == '\0');

        string = 'o';

        REQUIRE(string.Size() == 1);
        REQUIRE(string.Capacity() >= 2);
        REQUIRE(string[0] == 'o');
        REQUIRE(string.GetBuffer()[1] == '\0');
    }

    SECTION("String Equality")
    {
        string = "Test";
        Shipyard::StringA otherString = "test";

        REQUIRE(string != otherString);

        string[0] = 't';

        REQUIRE(string == otherString);

        otherString = "test2";

        REQUIRE(string != otherString);

        REQUIRE(otherString == "test2");
        REQUIRE(string != "test2");
    }

    SECTION("String Append")
    {
        string += "Test";

        REQUIRE(string.Size() == 4);
        REQUIRE(string.Capacity() >= 5);
        REQUIRE(string[0] == 'T');
        REQUIRE(string[1] == 'e');
        REQUIRE(string[2] == 's');
        REQUIRE(string[3] == 't');
        REQUIRE(string.GetBuffer()[4] == '\0');

        Shipyard::StringA otherString = "Some";

        otherString += string;

        REQUIRE(otherString == "SomeTest");

        otherString += "123";

        REQUIRE(otherString == "SomeTest123");

        otherString += '4';
        REQUIRE(otherString == "SomeTest1234");

        otherString.Append("56", 2);

        REQUIRE(otherString == "SomeTest123456");
    }

    SECTION("String Add")
    {
        string = "Test";

        string = "Some" + string;

        REQUIRE(string == "SomeTest");

        string = 'A' + string;

        REQUIRE(string == "ASomeTest");

        string = string + "123";

        REQUIRE(string == "ASomeTest123");

        string = string + '4';
        
        REQUIRE(string == "ASomeTest1234");
    }

    SECTION("String Assign")
    {
        string.Assign("Test123", 4);

        REQUIRE(string == "Test");
    }

    SECTION("String Insert")
    {
        string = "Some123";

        Shipyard::StringA otherString = "Test";

        string.Insert(4, otherString);

        REQUIRE(string == "SomeTest123");

        string.Insert(0, "A");

        REQUIRE(string == "ASomeTest123");

        string.Insert(string.Size(), "456", 2);

        REQUIRE(string == "ASomeTest12345");
    }

    SECTION("String Insert Substring")
    {
        string = "Some123";

        Shipyard::StringA otherString = "123Test456";

        string.InsertSubstring(4, otherString, 3, 4);

        REQUIRE(string == "SomeTest123");

        string.InsertSubstring(0, "A", 0, 1);

        REQUIRE(string == "ASomeTest123");

        string.InsertSubstring(string.Size(), "456", 0, 3);

        REQUIRE(string == "ASomeTest123456");
    }

    SECTION("String Erase")
    {
        string = "SomeTest123";

        string.Erase(0, 4);

        REQUIRE(string == "Test123");

        string.Erase(4, 3);

        REQUIRE(string == "Test");

        string.Erase(0, 100);

        REQUIRE(string == "");
    }

    SECTION("String Resize")
    {
        string.Resize(4);

        REQUIRE(string.Size() == 4);
        REQUIRE(string.Capacity() >= 5);
        REQUIRE(string.GetBuffer()[4] == '\0');

        memcpy(string.GetWriteBuffer(), "Test", 4);

        REQUIRE(string == "Test");

        const char* buffer = string.GetBuffer();

        string.Resize(3);

        REQUIRE(string.Size() == 3);
        REQUIRE(string.Capacity() >= 5);
        REQUIRE(string == "Tes");
        REQUIRE(string.GetBuffer() == buffer);

        string.Resize(0);

        REQUIRE(string.Size() == 0);
        REQUIRE(string.Capacity() >= 5);
        REQUIRE(string == "");
        REQUIRE(string.GetBuffer() == buffer);

        string += "abc";

        REQUIRE(string.Size() == 3);
        REQUIRE(string.Capacity() >= 5);
        REQUIRE(string == "abc");
        REQUIRE(string.GetBuffer() == buffer);

        string.Resize(6);

        REQUIRE(string.Size() == 6);
        REQUIRE(string.Capacity() >= 7);
        REQUIRE(string[0] == 'a');
        REQUIRE(string[1] == 'b');
        REQUIRE(string[2] == 'c');
        REQUIRE(string.GetBuffer()[6] == '\0');
        REQUIRE(string.GetBuffer() != buffer);
    }

    SECTION("String Reserve")
    {
        string.Reserve(4);

        REQUIRE(string.Size() == 0);
        REQUIRE(string.Capacity() == 4);
        
        const char* buffer = string.GetBuffer();

        string += "123";

        REQUIRE(string.Size() == 3);
        REQUIRE(string.Capacity() == 4);
        REQUIRE(string.GetBuffer() == buffer);
        REQUIRE(string == "123");

        string.Reserve(2);

        REQUIRE(string.Size() == 1);
        REQUIRE(string.Capacity() == 2);
        REQUIRE(string == "1");
        REQUIRE(string.GetBuffer() != buffer);

        string.Reserve(6);

        REQUIRE(string.Size() == 1);
        REQUIRE(string.Capacity() == 6);
        REQUIRE(string == "1");

        string += "23";

        REQUIRE(string.Size() == 3);
        REQUIRE(string.Capacity() == 6);
        REQUIRE(string == "123");
    }

    SECTION("String Clear")
    {
        string = "Test";

        string.Clear();

        REQUIRE(string.Size() == 0);
        REQUIRE(string.Capacity() == 1);
        REQUIRE(string == "");
    }

    SECTION("String FindIndexOfFirst")
    {
        string = "ThisIsATestString";

        Shipyard::StringA stringToFind = "this";

        REQUIRE(string.FindIndexOfFirst(stringToFind, 0) == Shipyard::StringA::InvalidIndex);
        
        stringToFind[0] = 'T';

        REQUIRE(string.FindIndexOfFirst(stringToFind, 0) == 0);

        REQUIRE(string.FindIndexOfFirst("is", 0) == 2);
        REQUIRE(string.FindIndexOfFirst("is", 2) == 2);
        REQUIRE(string.FindIndexOfFirst("is", 3) == Shipyard::StringA::InvalidIndex);

        REQUIRE(string.FindIndexOfFirst('S', 0) == 11);

        REQUIRE(string.FindIndexOfFirstCaseInsensitive("IS", 0) == 2);
        REQUIRE(string.FindIndexOfFirstCaseInsensitive("IS", 2) == 2);
        REQUIRE(string.FindIndexOfFirstCaseInsensitive("IS", 3) == 4);

        stringToFind = "teST";

        REQUIRE(string.FindIndexOfFirstCaseInsensitive(stringToFind, 0) == 7);

        REQUIRE(string.FindIndexOfFirstCaseInsensitive('a', 0) == 6);
    }

    SECTION("String Substring")
    {
        string = "ThisIsATestString";

        Shipyard::StringA substring = string.Substring(0, 4);

        REQUIRE(substring == "This");

        substring = string.Substring(6, 100);

        REQUIRE(substring == "ATestString");
    }

    SECTION("String Compare")
    {
        string = "Test";

        Shipyard::StringA otherString = "test";

        REQUIRE(string.Compare(otherString) < 0);
        REQUIRE(otherString.Compare(string) > 0);
        REQUIRE(string.CompareCaseInsensitive(otherString) == 0);
        REQUIRE(otherString.CompareCaseInsensitive(string) == 0);

        string[0] = 't';

        REQUIRE(string.Compare(otherString) == 0);
        REQUIRE(otherString.Compare(string) == 0);
        REQUIRE(string.CompareCaseInsensitive(otherString) == 0);
        REQUIRE(otherString.CompareCaseInsensitive(string) == 0);

        string = "test2";

        REQUIRE(string.Compare(otherString) > 0);
        REQUIRE(otherString.Compare(string) < 0);
        REQUIRE(string.CompareCaseInsensitive(otherString) > 0);
        REQUIRE(otherString.CompareCaseInsensitive(string) < 0);
    }
}