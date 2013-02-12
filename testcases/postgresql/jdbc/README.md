To run: 

COMMANDLINE
-----------

Install Apache Maven if it isn't already installed, then:

    mvn -Dtest.url=jdbc:postgresql:regress -Dtest.username=postgres -Dtest.password=postgres test

replacing "regress" with your test DB name, and the specifying a superuser name
and password. A superuser is needed to run the tests that convert the casts to
implicit; if you don't wnat to run that section of the tests, just use a normal
user account and ignore the failure of ImplicitCastsTest.

Expected failures are for NoImplicitTest cases where the strict cast checking
is preventing text to json/xml conversion:

    testJSONFuncSetObjectVarchar
    testJSONFuncSetObjectDetect
    testJSONFuncSetString
    testXMLFuncSetObjectVarchar
    testXMLFuncSetObjectDetect
    testXMLFuncSetString

all of which fail with ERROR: function testjson(character varying) does not
exist. The corresponding versions in ImplicitCastsTest should succeed.


ECLIPSE
-------

Alternately, open as a Maven project in Eclipse with m2e installed. Get
properties on the project. Go to "run/debug settings". Click "New..." and
choose "Junit". In the "arguments" tab set "vm arguments" to:

    -Dtest.username=me -Dtest.password=mypw -Dtest.url=jdbc:postgresql:mydb

where "me", "mypw" and "mydb" are as explained in the command line maven info
section.
