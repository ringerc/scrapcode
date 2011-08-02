package com.mycompany.leaktest2;

import java.util.logging.Level;


public class CustomLevel extends java.util.logging.Level {
    
    public static final Level CUSTOM_LEVEL = new CustomLevel();
    
    private CustomLevel() {
        super("CustomLevel",99);
    }
    
}
