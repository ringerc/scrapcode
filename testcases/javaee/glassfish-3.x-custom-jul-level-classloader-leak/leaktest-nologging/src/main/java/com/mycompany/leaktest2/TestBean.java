package com.mycompany.leaktest2;

import java.util.logging.Level;
import java.util.logging.Logger;
import javax.annotation.ManagedBean;
import javax.faces.bean.RequestScoped;

public class TestBean {
    
    public String getHello() {
        // Using a standard log level is fine, because the reference is to another
        // JDK class not a class loaded by the webapp class loader.
        Logger.getLogger(TestBean.class.getName()).log(Level.INFO, "Sending standard log message");
        return "Hello";
    }
    
    
}
