package id.au.ringerc.testcase.as7.eclipselink;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Reader;
import java.io.Writer;
import java.nio.CharBuffer;

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

import org.eclipse.persistence.Version;
import org.jboss.shrinkwrap.api.ShrinkWrap;
import org.jboss.shrinkwrap.api.formatter.Formatter;
import org.jboss.shrinkwrap.api.formatter.Formatters;
import org.jboss.shrinkwrap.api.spec.JavaArchive;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.junit.Assert;
import org.junit.Test;

/**
 * The base class for all the test variants performs exactly the same series
 * of tests for each variant. The variants control the environment and
 * configuration by deploying a different persistence.xml file or making
 * other changes.
 * 
 * @author Craig Ringer <ringerc@ringerc.id.au>
 *
 */
class TestBase {
	
	private static final boolean verbose = false;

	protected static WebArchive makeDeployment(String persistenceXmlName) throws IOException {
		File persistenceXml = new File( "src/main/java/META-INF/", persistenceXmlName);
		WebArchive jar = ShrinkWrap.create(WebArchive.class)
				.addAsResource(persistenceXml, "META-INF/persistence.xml")
				.addAsWebInfResource(new File("src/main/java/META-INF/beans.xml"), "beans.xml")
				.addClasses(
						DBProvider.class, DummyEntity.class, DummyEntity_.class, DummyEJB.class, TestBase.class,
						JBossAS7ServerPlatform.class, JBossAS7ServerPlatform.JBossAS7TransactionController.class,
						JBossLogger.class);
		if (verbose) {
			printPersistenceXml(persistenceXml);
			System.err.println("\n---");
			jar.writeTo(System.err, Formatters.VERBOSE);
			System.err.println("\n---\n");
		}
		return jar;
	}

	
	@Inject
	private DummyEJB dummyEJB;
	
	protected void checkEclipseVersion() {
		System.err.println("ECLIPSE System version is: " + Version.getVersion());
	}
	
	protected void ensureInjected() {
		assertNotNull(dummyEJB);
	}

	protected void staticMetaModelWorks() {
		try {
			DummyEntity_.id.getName();
		} catch (NullPointerException ex) {
			Assert.fail("Static metamodel has null members: " + ex);
		}
	}

	protected void isTransactional() {
		try {
			dummyEJB.failsIfNotTransactional();
		} catch (javax.persistence.TransactionRequiredException ex) {
			Assert.fail("Operation failed with " + ex);
		} catch (javax.ejb.EJBException ex) {
			Assert.fail("Operation failed with " + ex);
		}
	}

	protected void dynamicMetaModelWorks() {
		try {
			dummyEJB.dynamicMetaModelWorks();
		} catch (java.lang.IllegalArgumentException ex) {
			Assert.fail("Unable to resolve metamodel class " + ex);
		} catch (javax.ejb.EJBException ex) {
			Assert.fail("Unable to resolve metamodel class " + ex);
		}
	}
	
	protected void databaseAccessWorks() {
		try {
			dummyEJB.createDummy(1);
			DummyEntity e = dummyEJB.getDummy(1);
			dummyEJB.deleteDummy(e);
		} catch (java.lang.IllegalArgumentException ex) {
			Assert.fail("Basic entity access failed " + ex);
		} catch (javax.ejb.EJBException ex) { 
			Assert.fail("Basic entity access failed " + ex);
		}
	}
	
	private static void printPersistenceXml(File persistenceXml) throws IOException {
		System.err.println("\nEclipse build-side version: " + Version.getVersion());
		System.err.println("\nUsing persistence.xml: " + persistenceXml);
		
		Writer w = new OutputStreamWriter(System.err);
		Reader r = new FileReader(persistenceXml);
		char[] buf = new char[1024];
		int bytesRead = 0;
		while ( (bytesRead = r.read(buf)) > 0 ) {
			w.write(buf, 0, bytesRead);
		}
		
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
			Attribute<? super DummyEntity, ?> y = x.getAttribute("dummy");
			if (!String.class.equals(y.getJavaType())) {
				throw new AssertionError("Unexpected class");
			}
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
			e = em.merge(e);
			em.remove(e);
		}
		
	}

}
