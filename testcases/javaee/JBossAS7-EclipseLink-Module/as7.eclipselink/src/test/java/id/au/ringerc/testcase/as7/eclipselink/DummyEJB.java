package id.au.ringerc.testcase.as7.eclipselink;

import java.util.logging.Logger;

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
import javax.persistence.metamodel.Metamodel;

@Stateless
public class DummyEJB {

	private static final Logger logger = Logger.getLogger(DummyEJB.class.getName());
	
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
		logger.info("Before get metamodel, DummyEntity_.id is " + DummyEntity_.id);
		Metamodel mm = em.getMetamodel();
		logger.info("After get metamodel, DummyEntity_.id is " + DummyEntity_.id);
		EntityType<DummyEntity> me = mm.entity(DummyEntity.class);
		logger.info("After get entity, DummyEntity_.id is " + DummyEntity_.id);
		Attribute<? super DummyEntity, ?> attr = me.getAttribute("dummy");
		logger.info("After get attribute, DummyEntity_.id is " + DummyEntity_.id);
		if (!String.class.equals(attr.getJavaType())) {
			throw new AssertionError("Unexpected class");
		}
		logger.info("After get java type on attribute, DummyEntity_.id is " + DummyEntity_.id);
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