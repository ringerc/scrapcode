package com.example.shrinkwrapwarbug;

import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import org.jboss.shrinkwrap.api.ShrinkWrap;
import org.jboss.shrinkwrap.api.spec.JavaArchive;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.junit.*;
import static org.junit.Assert.*;
import org.junit.runner.RunWith;

@RunWith(Arquillian.class)
public class DemoTest {

    @Deployment
    public static WebArchive createDeployment() {
        // Note the file name: .jar not .war
        //
        // Arquillian still happily deploys it to the server, but the server
        // can't find the classes, causing wacky error messages.
        //
        return ShrinkWrap.create(WebArchive.class, "demo.jar")
                .addClass(Demo.class);
    }
    
    @Test
    public void testReturnOne() {
        Demo d = new Demo();
        assertEquals(d.returnOne(), 1);
    }
    
}
