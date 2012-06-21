package id.au.ringerc.testcase.as7.eclipselink;

import static org.junit.Assert.assertNotNull;

import javax.inject.Inject;

import id.au.ringerc.testcase.as7.eclipselink.entities.DummyEntity;
import id.au.ringerc.testcase.as7.eclipselink.entities.DummyEntity_;

import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(Arquillian.class)
public class IncludeUnlistedClassesTest extends TestBase {

	@Deployment
	public static WebArchive makeDeployment() {
		return TestBase.makeDeployment("persistence.xml");
	}

	@Inject
	private DummyEJB dummyEJB;
	
	@Test
	public void ensureInjected() {
		assertNotNull(dummyEJB);
	}

	@Test
	public void staticMetaModelWorks() {
		assertNotNull(DummyEntity_.id);
	}

	@Test
	public void isTransactional() {
		dummyEJB.failsIfNotTransactional();
	}

	@Test
	public void dynamicMetaModelWorks() {
		dummyEJB.dynamicMetaModelWorks();
	}
	
	@Test
	public void classDiscoveryWorks() {
		dummyEJB.createDummy(1);
		DummyEntity e = dummyEJB.getDummy(1);
		dummyEJB.deleteDummy(e);
	}
	

}
