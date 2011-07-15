package com.example;

import javax.ejb.Stateless;

@Stateless
public class ConcreteEJBSubclass extends AbstractBase {
    
    // If the inherited doSomething() method is overridden with a wrapper like
    //
    // @Override
    // public String doSomething() { return super.doSomething() };
    // 
    // the error is worked around.

}
