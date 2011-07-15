package com.example;

import javax.ejb.Stateless;

@Stateless
public class ConcreteEJBSubclass extends AbstractBase<String> {
    
    // We do *not* override doSomething()
    
}
