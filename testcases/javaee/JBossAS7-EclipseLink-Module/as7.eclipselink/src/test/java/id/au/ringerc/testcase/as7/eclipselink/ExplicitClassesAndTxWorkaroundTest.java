package id.au.ringerc.testcase.as7.eclipselink;

import id.au.ringerc.testcase.as7.eclipselink.entities.DummyEntity_;

import java.io.IOException;
import java.util.logging.Logger;

import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;

/**
 * This test uses two workarounds.
 * 
 * First, entity classes are explicitly listed in persistence.xml , as in ExplicitClassesTest.
 * 
 * Second, it adds a custom server adapter JBossAS7ServerPlatform and sets it as the 
 * eclipselink.target-server in persistence.xml so that EclipseLink can look resources
 * up correctly in JNDI. That's supposed to be fixed by:
 *   https://bugs.eclipse.org/bugs/show_bug.cgi?id=365704
 * but doesn't appear to be fixed.
 * 
 * The dynamic metamodel is populated and classes are discoverable, but
 * EclipseLink hasn't enriched the static metamodel so staticMetaModelWorks
 * will still fail.
 *
 * @author Craig Ringer <ringerc@ringerc.id.au>
 *
 */
@RunWith(Arquillian.class)
@Ignore
public class ExplicitClassesAndTxWorkaroundTest extends TestBase {
	
	private static final Logger logger = Logger.getLogger(ExplicitClassesAndTxWorkaroundTest.class.getName());

	@Deployment
	public static WebArchive makeDeployment() throws IOException {
		return TestBase.makeDeployment("explicit-and-txaround-persistence.xml", "explicit-classes-and-tx-workaround");
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
		super.staticMetaModelWorks();
	}

	// Works, bizarrely. See https://bugs.eclipse.org/bugs/show_bug.cgi?id=383199
	@Test
	public void staticMetamodelWorksAfterDynamicModelAccess() {
		logger.info("Before dynamic metamodel access, DummyEntity_.id is " + DummyEntity_.id);
		Assert.assertNull(DummyEntity_.id);
		super.dynamicMetaModelWorks();
		Assert.assertNotNull(DummyEntity_.id);
		logger.info("After dynamic metamodel access, DummyEntity_.id is " + DummyEntity_.id);
	}

	// Works because we've explicitly listed classes
	@Test
	public void dynamicMetaModelWorks() {
		super.dynamicMetaModelWorks();
	}
	
	// Succeeds, we worked around transaction management issues
	@Test
	public void isTransactional() {
		super.isTransactional();
	}

	// Succeeds, we've worked around transaction management issues
	@Test
	@Ignore
	public void databaseAccessWorks() {
		super.databaseAccessWorks();
	}
	

}
