package id.au.ringerc.testcase.as7.eclipselink;

import static org.junit.Assert.assertNotNull;

import java.io.IOException;

import javax.inject.Inject;

import id.au.ringerc.testcase.as7.eclipselink.entities.DummyEntity;
import id.au.ringerc.testcase.as7.eclipselink.entities.DummyEntity_;

import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import org.jboss.shrinkwrap.api.spec.WebArchive;
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
public class ExplicitClassesAndTxWorkaroundTest extends TestBase {

	@Deployment
	public static WebArchive makeDeployment() throws IOException {
		return TestBase.makeDeployment("explicit-and-txaround-persistence.xml");
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

	// Succeeds, we worked around transaction management issues
	@Test
	public void isTransactional() {
		super.isTransactional();
	}

	// Works because we've explicitly listed classes
	@Test
	public void dynamicMetaModelWorks() {
		super.dynamicMetaModelWorks();
	}
	
	// Succeeds, we've worked around transaction management issues
	@Test
	public void databaseAccessWorks() {
		super.databaseAccessWorks();
	}
	

}
