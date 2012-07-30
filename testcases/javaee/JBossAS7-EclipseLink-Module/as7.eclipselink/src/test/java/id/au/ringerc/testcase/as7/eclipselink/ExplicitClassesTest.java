package id.au.ringerc.testcase.as7.eclipselink;

import java.io.IOException;
import java.util.logging.Logger;

import id.au.ringerc.testcase.as7.eclipselink.entities.DummyEntity_;

import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;

/**
 * ExplicitClassesTest adds a workaround for EclipseLink's apparent
 * inability to discover entity classes by annotation scanning at
 * runtime by listing classes explicitly in persistence.xml.
 * 
 * Transactional methods still won't work because of
 *    https://bugs.eclipse.org/bugs/show_bug.cgi?id=365704
 * 
 * The dynamic metamodel is populated and classes are discoverable, but
 * EclipseLink hasn't enriched the static metamodel so staticMetaModelWorks
 * will still fail.
 *
 * @author Craig Ringer <ringerc@ringerc.id.au>
 */
@RunWith(Arquillian.class)
public class ExplicitClassesTest extends TestBase {
	
	private static final Logger logger = Logger.getLogger(ExplicitClassesTest.class.getName());

	@Deployment
	public static WebArchive makeDeployment() throws IOException {
		return TestBase.makeDeployment("explicit-classes-persistence.xml", "explicit-classes");
	}

	// Informational only, prints server-side EclipseLink verison to logs.
	@Test
	@Ignore
	public void checkEclipseVersion() {
		super.checkEclipseVersion();
	}
	
	// Always succeeds, just making sure CDI is happy
	@Test
	@Ignore
	public void ensureInjected() {
		super.ensureInjected();
	}

	// Expects to fail; metamodel not enriched
	@Test
	public void staticMetaModelWorks() {
		logger.info("Static metamodel test");
		super.staticMetaModelWorks();
	}
	
	@Test
	public void staticMetamodelWorksAfterTrivialEntityManagerCall() {
		super.staticMetamodelWorksAfterTrivialEntityManagerCall();
	}

	// Works, bizarrely. See https://bugs.eclipse.org/bugs/show_bug.cgi?id=383199
	@Test
	public void staticMetamodelWorksAfterDynamicModelAccess() {
		logger.info("Dynamic-then-static metamodel test");
		logger.info("Before dynamic metamodel access, DummyEntity_.id is " + DummyEntity_.id);
		Assert.assertNull(DummyEntity_.id);
		super.dynamicMetaModelWorks();
		Assert.assertNotNull(DummyEntity_.id);
		logger.info("After dynamic metamodel access, DummyEntity_.id is " + DummyEntity_.id);
	}

	// Works, metamodel populated when classes
	// listed explicitly.
	@Test
	public void dynamicMetaModelWorks() {
		logger.info("Dynamic metamodel test");
		super.dynamicMetaModelWorks();
	}
	
	// Expects to fail; TransactionManager not found
	@Test
	public void isTransactional() {
		super.isTransactional();
	}
	
	// Fails, no transaction management.
	@Test
	@Ignore
	public void databaseAccessWorks() {
		super.databaseAccessWorks();
	}

}
