package com.mycompany.glassfishseamfacesflashinjection;

import javax.faces.context.Flash;
import javax.inject.Inject;

public class InjectionTest {
    
    // If the flash scope isn't injected anywhere, Weld never detects the ambiguous
    // injection conflict.
    @Inject
    private Flash flash;

}
