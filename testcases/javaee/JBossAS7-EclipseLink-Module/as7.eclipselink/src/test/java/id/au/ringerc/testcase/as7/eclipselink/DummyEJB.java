package id.au.ringerc.testcase.as7.eclipselink;

import id.au.ringerc.testcase.as7.eclipselink.entities.DummyEntity;

import javax.ejb.Stateless;
import javax.ejb.TransactionAttribute;
import javax.ejb.TransactionAttributeType;
import javax.inject.Inject;
import javax.persistence.EntityManager;
import javax.persistence.Query;
import javax.persistence.metamodel.Attribute;
import javax.persistence.metamodel.EntityType;

@Stateless
public class DummyEJB {

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