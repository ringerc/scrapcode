package id.au.ringerc.testcase.as7.eclipselink;

import id.au.ringerc.testcase.as7.eclipselink.entities.DummyEntity_;

import java.io.IOException;

import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import java.util.logging.Logger;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;

/**
 * This base test class uses a regular persistence.xml . It doesn't
 * explicitly list persistent classes, relying on
 * include-unlist-classes, and it doesn't set a custom server
 * adapter.
 * 
 * Most of these tests will fail.
 * 
 * @author Craig Ringer <ringerc@ringerc.id.au>
 */
@RunWith(Arquillian.class)
public class NoWorkaroundsTest extends TestBase {

	private static final Logger logger = Logger.getLogger(NoWorkaroundsTest.class.getName());

	@Deployment
	public static WebArchive makeDeployment() throws IOException {
		return TestBase.makeDeployment("persistence.xml");
	}

	// Informational only, prints server-side EclipseLink verison to logs.
	@Test
	public void checkEclipseVersion() {
		super.checkEclipseVersion();
	}

	// Always succeeds, just making sure CDI is happy
	@Test
	public void ensureInjected() {
		super.ensureInjected();
	}

	// Expects to fail; metamodel not enriched
	@Test
	public void staticMetaModelWorks() {
		super.staticMetaModelWorks();
	}

	// Expected to fail; no entities known for metamodel
	@Test
	public void staticMetamodelWorksAfterDynamicModelAccess() {
		logger.info("Before dynamic metamodel access, DummyEntity_.id is " + DummyEntity_.id);
		Assert.assertNull(DummyEntity_.id);
		super.dynamicMetaModelWorks();
		Assert.assertNotNull(DummyEntity_.id);
		logger.info("After dynamic metamodel access, DummyEntity_.id is " + DummyEntity_.id);
	}

	// Will fail, EclipseLink can't find model classes
	@Test
	public void dynamicMetaModelWorks() {
		super.dynamicMetaModelWorks();
	}

	// Will fail, EclipseLink can't find the transaction manager
	@Test
	public void isTransactional() {
		super.isTransactional();
	}

	// Will fail, EclipseLink can't find the transaction manager
	// and can't find the entity classes anyway.
	@Test
	public void databaseAccessWorks() {
		super.databaseAccessWorks();
	}
	

}
