package com.example;

// Doesn't actually matter for the purpose of this test
// whether or not AbstractBase is really abstract, the test case
// fails either way.
//
// Unit tests with both abstract and non-abstract versions should be added,
// as well as a version where AbstractBase is generic and uses that generic
// type in both method argument and return type.
//
public class AbstractBase {
    
    public String doSomething() {
        return "Something";
    }
    
}