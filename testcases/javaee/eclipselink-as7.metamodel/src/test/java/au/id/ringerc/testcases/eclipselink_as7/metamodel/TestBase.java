package au.id.ringerc.testcases.eclipselink_as7.metamodel;

import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import junit.framework.Assert;

import org.jboss.shrinkwrap.api.ShrinkWrap;
import org.jboss.shrinkwrap.api.spec.WebArchive;

public class TestBase {
	
	//
	// If true, tests will pass when the wrong - but expected by this test - behaviour of AS7 + EClipseLink is seen
	//
	// If false, tests will pass when the spec-correct behaviour is seen. This should pass with Hibernate as the provider.
	//
	boolean assertExpectedButIncorrect = true;
	
	protected void expectNullBefore() {
		if (assertExpectedButIncorrect) {
			// We're verifying that the metamodel is in the expected but INCORRECT
			// state
			if (DummyEntity_.id != null) {
				throw new IllegalStateException("Entity metamodel fields non-null before test where null expected");
			}
		} else {
			// We're asserting the _correct_ behaviour
			Assert.assertNotNull(DummyEntity_.id);
		}
	}

	protected void expectNotNullAfter() {
		Assert.assertNotNull(DummyEntity_.id);
	}

	protected void expectNullAfter() {
		if (assertExpectedButIncorrect) {
			// We're asserting that certain methods did NOT trigger population of the metamodel
			Assert.assertNull(DummyEntity_.id);
		}
	}
	
	protected static WebArchive makeDeployment(String name) {
		if (!name.endsWith(".war")) {
			throw new IllegalArgumentException("Only archive names ending in .war are valid");
		}
		return ShrinkWrap.create(WebArchive.class, name)
				.addAsResource(new File("src/main/java/META-INF/persistence.xml"), "META-INF/persistence.xml")
				.addAsWebInfResource(new File("src/main/java/META-INF/beans.xml"), "beans.xml")
				.addClasses(DummyEntity.class, DummyEntity_.class, TestBase.class);
	}
	
}