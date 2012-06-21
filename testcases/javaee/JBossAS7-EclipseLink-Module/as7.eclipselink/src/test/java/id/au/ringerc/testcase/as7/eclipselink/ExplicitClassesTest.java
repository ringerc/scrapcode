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

	@Deployment
	public static WebArchive makeDeployment() throws IOException {
		return TestBase.makeDeployment("explicit-classes-persistence.xml");
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

	// Expects to fail; TransactionManager not found
	@Test
	public void isTransactional() {
		super.isTransactional();
	}

	// Works, metamodel populated when classes
	// listed explicitly.
	@Test
	public void dynamicMetaModelWorks() {
		super.dynamicMetaModelWorks();
	}
	
	// Fails, no transaction management.
	@Test
	public void databaseAccessWorks() {
		super.databaseAccessWorks();
	}
	

}
