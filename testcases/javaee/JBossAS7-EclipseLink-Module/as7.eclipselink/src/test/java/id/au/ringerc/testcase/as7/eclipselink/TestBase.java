package id.au.ringerc.testcase.as7.eclipselink;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.io.File;

import id.au.ringerc.testcase.as7.eclipselink.entities.DummyEntity;
import id.au.ringerc.testcase.as7.eclipselink.entities.DummyEntity_;

import javax.ejb.Stateless;
import javax.ejb.TransactionAttribute;
import javax.ejb.TransactionAttributeType;
import javax.inject.Inject;
import javax.persistence.EntityManager;
import javax.persistence.Query;
import javax.persistence.metamodel.Attribute;
import javax.persistence.metamodel.EntityType;

import org.jboss.shrinkwrap.api.ShrinkWrap;
import org.jboss.shrinkwrap.api.formatter.Formatter;
import org.jboss.shrinkwrap.api.formatter.Formatters;
import org.jboss.shrinkwrap.api.spec.JavaArchive;
import org.jboss.shrinkwrap.api.spec.WebArchive;

class TestBase {

	protected static WebArchive makeDeployment(String persistenceXmlName) {
		System.err.println("\n---");
		WebArchive jar = ShrinkWrap.create(WebArchive.class)
				.addAsResource(new File("src/main/java/META-INF/",persistenceXmlName), "META-INF/persistence.xml")
				.addAsWebInfResource(new File("src/main/java/META-INF/beans.xml"), "beans.xml")
				.addClasses(DBProvider.class, DummyEntity.class, DummyEntity_.class, DummyEJB.class, TestBase.class);
		jar.writeTo(System.err, Formatters.VERBOSE);
		System.err.println("\n---\n");
		return jar;
	}
	
	@Stateless
	public static class DummyEJB {

		@Inject
		private EntityManager em;
		
		@TransactionAttribute(TransactionAttributeType.REQUIRED)
		public void failsIfNotTransactional() {
			Query q;
			q = em.createNativeQuery("SAVEPOINT txtest");
			q.executeUpdate();
			q = em.createNativeQuery("ROLLBACK TO SAVEPOINT txtest");
			q.executeUpdate();
		}
		
		public void dynamicMetaModelWorks() {
			EntityType<DummyEntity> x = em.getMetamodel().entity(DummyEntity.class);
			assertNotNull(x);
			Attribute<? super DummyEntity, ?> y = x.getAttribute("dummy");
			assertNotNull(y);
			assertEquals(String.class, y.getJavaType());
		}
		
		public DummyEntity createDummy(Integer id) {
			DummyEntity e = new DummyEntity(id);
			em.persist(e);
			return e;
		}
		
		public DummyEntity getDummy(Integer id) {
			return em.find(DummyEntity.class, id);
		}
		
		public void deleteDummy(DummyEntity e) {
			em.remove(e);
		}
		
	}

}
