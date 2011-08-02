package org.example.elexpressionsoncustomcollections;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import javax.enterprise.context.RequestScoped;
import javax.inject.Named;

@Named
@RequestScoped
public class TestBean {
    
    public class Wrap {
        private String string;
        public Wrap() { string = null; }
        public Wrap(String string) { this.string = string; }
        public String getString() { return string;  }
        public void setString(String string) { this.string = string; }
        @Override
        public String toString() { return string; }
    }
    
    public enum TypeSelect {
        ArrayList,
        UnmodifiableList,
        SynchronizedList,
        HashSet,
        UnmodifiableSet,
        SynchronizedSet;
    }
    
    private final List<Wrap> testList = new ArrayList<Wrap>();
    private final Set<Wrap> testSet = new HashSet<Wrap>();
    private TypeSelect collectionType = TypeSelect.ArrayList;
    
    public TestBean() {
        testList.add(new Wrap("one"));
        testList.add(new Wrap("two"));
        testList.add(new Wrap("three"));
        testSet.addAll(testList);
    }
    
    public TypeSelect getCollectionType() {
        return collectionType;
    }
    
    public void setCollectionType(TypeSelect t) {
        this.collectionType = t;
    }
    
    public TypeSelect[] getCollectionTypes() {
        return TypeSelect.values();
    }
    
    public Collection<Wrap> getCollection() {
        switch (collectionType) {
            case ArrayList:
                return testList;
            case HashSet:
                return testSet;
            case SynchronizedList:
                return Collections.synchronizedList(testList);
            case SynchronizedSet:
                return Collections.synchronizedSet(testSet);
            case UnmodifiableList:
                return Collections.unmodifiableList(testList);
            case UnmodifiableSet: 
                return Collections.unmodifiableSet(testSet);
            default:
                throw new AssertionError("Unhandled case");
        }
    }
    
}
