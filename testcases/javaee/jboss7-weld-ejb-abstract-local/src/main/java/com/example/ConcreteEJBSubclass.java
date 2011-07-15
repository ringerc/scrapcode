package com.example;

import javax.ejb.Stateless;

@Stateless
public class ConcreteEJBSubclass extends AbstractBase<String> {
    
    @Override
    public String getMsg() {
        return "Hello";
    }
    
    // We do *not* override doSomething()
    
}
