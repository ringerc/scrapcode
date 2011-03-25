Deployment log and failure for this test case, deploying to Glassfish 3.1:

WARNING: RAR8068: Using default datasource : __ds_jdbc_ra for pool : classads-database-pool
INFO: Instantiated an instance of org.hibernate.validator.engine.resolver.JPATraversableResolver.
WARNING: RAR8068: Using default datasource : __ds_jdbc_ra for pool : classads-database-pool
CONFIG: The access type for the persistent class [class au.com.postnewspapers.classads.persistence.entity.Customer] is set to [FIELD].
CONFIG: The access type for the persistent class [class au.com.postnewspapers.classads.persistence.entity.Classification] is set to [FIELD].
CONFIG: The target entity (reference) class for the many to one mapping element [field classadSection] is being defaulted to: class au.com.postnewspapers.classads.persistence.entity.ClassadSection.
CONFIG: The access type for the persistent class [class au.com.postnewspapers.classads.persistence.entity.Ad] is set to [FIELD].
CONFIG: The target entity (reference) class for the many to one mapping element [field customer] is being defaulted to: class au.com.postnewspapers.classads.persistence.entity.Customer.
CONFIG: The target entity (reference) class for the many to one mapping element [field classification] is being defaulted to: class au.com.postnewspapers.classads.persistence.entity.Classification.
CONFIG: The target class (reference) class for the element collection mapping element [field insertionCollection] is being defaulted to: class au.com.postnewspapers.classads.persistence.entity.Insertion.
CONFIG: The access type for the persistent class [class au.com.postnewspapers.classads.persistence.entity.ClassadSection] is set to [FIELD].
CONFIG: The access type for the persistent class [class au.com.postnewspapers.classads.persistence.entity.Insertion] is set to [FIELD].
CONFIG: The access type for the persistent class [class au.com.postnewspapers.classads.persistence.entity.ChangeTrackFields] is set to [FIELD].
CONFIG: The alias name for the entity class [class au.com.postnewspapers.classads.persistence.entity.Customer] is being defaulted to: Customer.
CONFIG: The alias name for the entity class [class au.com.postnewspapers.classads.persistence.entity.Classification] is being defaulted to: Classification.
CONFIG: The alias name for the entity class [class au.com.postnewspapers.classads.persistence.entity.Ad] is being defaulted to: Ad.
CONFIG: The alias name for the entity class [class au.com.postnewspapers.classads.persistence.entity.ClassadSection] is being defaulted to: ClassadSection.
CONFIG: The primary key column name for the mapping element [field insertionCollection] is being defaulted to: id.
CONFIG: Class au.com.postnewspapers.classads.persistence.entity.Ad could not be weaved for change tracking as it is not supported by its mappings.
INFO: TopLevel AvailabilityService.getAvailabilityEnabled => true
INFO: TopLevel EjbAvailabilityService.getAvailabilityEnabled => true
INFO: **Global AvailabilityEnabled => true; isAppHAEnabled: false
INFO: StatefulContainerBuilder AvailabilityEnabled for this app => false
INFO: StatefulContainerBuilder.buildStoreManager() storeName: AdSearchFacade-85263460990648321-BackingStore
INFO: [FileBackingStore::initialize] Successfully Created and initialized store. Working dir: C:\glassfish31\glassfish\domains\domain1\session-store\AdSearchFacade-85263460990648321; Configuration: BackingStoreConfiguration{clusterName='null', instanceName='null', storeName='AdSearchFacade-85263460990648321-BackingStore', shortUniqueName='85263460990648321', storeType='file', maxIdleTimeInSeconds=-1, relaxVersionCheck='null', maxLoadWaitTimeInSeconds=0, baseDirectoryName='C:\glassfish31\glassfish\domains\domain1\session-store\AdSearchFacade-85263460990648321', keyClazz=interface java.io.Serializable, valueClazz=class org.glassfish.ha.store.util.SimpleMetadata, synchronousSave=false, typicalPayloadSizeInKiloBytes=0, vendorSpecificSettings={start.gms=false, async.replication=true, key.transformer=com.sun.ejb.base.sfsb.util.SimpleKeyGenerator@51c53f, local.caching=true, value.class.is.thread.safe=true, broadcast.remove.expired=false}}
WARNING: StatefulContainerbuilder instantiated store: org.glassfish.ha.store.adapter.file.FileBackingStore@6cd344; ha-enabled: false ==> BackingStoreConfiguration{clusterName='null', instanceName='null', storeName='AdSearchFacade-85263460990648321-BackingStore', shortUniqueName='85263460990648321', storeType='file', maxIdleTimeInSeconds=-1, relaxVersionCheck='null', maxLoadWaitTimeInSeconds=0, baseDirectoryName='C:\glassfish31\glassfish\domains\domain1\session-store\AdSearchFacade-85263460990648321', keyClazz=interface java.io.Serializable, valueClazz=class org.glassfish.ha.store.util.SimpleMetadata, synchronousSave=false, typicalPayloadSizeInKiloBytes=0, vendorSpecificSettings={start.gms=false, async.replication=true, key.transformer=com.sun.ejb.base.sfsb.util.SimpleKeyGenerator@51c53f, local.caching=true, value.class.is.thread.safe=true, broadcast.remove.expired=false}}
INFO: Portable JNDI names for EJB AdSearchFacade : [java:global/au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT/AdSearchFacade!au.com.postnewspapers.classads.persistence.facade.AdSearchFacade, java:global/au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT/AdSearchFacade]
INFO: Portable JNDI names for EJB ClassadSectionFacade : [java:global/au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT/ClassadSectionFacade, java:global/au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT/ClassadSectionFacade!au.com.postnewspapers.classads.persistence.facade.ClassadSectionFacade]
INFO: Portable JNDI names for EJB AdFacade : [java:global/au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT/AdFacade!au.com.postnewspapers.classads.persistence.facade.AdFacade, java:global/au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT/AdFacade]
INFO: Portable JNDI names for EJB ClassificationFacade : [java:global/au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT/ClassificationFacade, java:global/au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT/ClassificationFacade!au.com.postnewspapers.classads.persistence.facade.ClassificationFacade]
INFO: Portable JNDI names for EJB EditorMarkupTransformer : [java:global/au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT/EditorMarkupTransformer!au.com.postnewspapers.classads.webui.EditorMarkupTransformer, java:global/au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT/EditorMarkupTransformer]
INFO: Portable JNDI names for EJB CustomerFacade : [java:global/au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT/CustomerFacade, java:global/au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT/CustomerFacade!au.com.postnewspapers.classads.persistence.facade.CustomerFacade]
INFO: Instantiated an instance of org.hibernate.validator.engine.resolver.JPATraversableResolver.
INFO: Initializing Mojarra 2.1.0 (FCS 2.1.0-b11) for context '/classads-webui-1.0-SNAPSHOT'
INFO: Monitoring jndi:/server/classads-webui-1.0-SNAPSHOT/WEB-INF/faces-config.xml for modifications
INFO: WEB0671: Loading application [au.com.postnewspapers.classads_classads-webui_war_1.0-SNAPSHOT] at [/classads-webui-1.0-SNAPSHOT]
SEVERE: Exception while loading the app
SEVERE: Exception while loading the app : WELD-001409 Ambiguous dependencies for type [Flash] with qualifiers [@Default] at injection point [[field] @Inject private transient au.com.postnewspapers.classads.webui.AdSearchController.flashScope]. Possible dependencies [[Producer Method [Flash] with qualifiers [@Any @Default] declared as [[method] @Produces @RequestScoped public org.jboss.seam.faces.environment.FacesFlashProducer.getFlash()], Producer Method [Flash] with qualifiers [@Any @Default] declared as [[method] @Produces @RequestScoped public org.jboss.seam.faces.context.FlashProducer.getFlash()]]]
org.jboss.weld.exceptions.DeploymentException: WELD-001409 Ambiguous dependencies for type [Flash] with qualifiers [@Default] at injection point [[field] @Inject private transient au.com.postnewspapers.classads.webui.AdSearchController.flashScope]. Possible dependencies [[Producer Method [Flash] with qualifiers [@Any @Default] declared as [[method] @Produces @RequestScoped public org.jboss.seam.faces.environment.FacesFlashProducer.getFlash()], Producer Method [Flash] with qualifiers [@Any @Default] declared as [[method] @Produces @RequestScoped public org.jboss.seam.faces.context.FlashProducer.getFlash()]]]
	at org.jboss.weld.bootstrap.Validator.validateInjectionPoint(Validator.java:309)
	at org.jboss.weld.bootstrap.Validator.validateBean(Validator.java:139)
	at org.jboss.weld.bootstrap.Validator.validateRIBean(Validator.java:162)
	at org.jboss.weld.bootstrap.Validator.validateBeans(Validator.java:385)
	at org.jboss.weld.bootstrap.Validator.validateDeployment(Validator.java:371)
	at org.jboss.weld.bootstrap.WeldBootstrap.validateBeans(WeldBootstrap.java:390)
	at org.glassfish.weld.WeldDeployer.event(WeldDeployer.java:190)
	at org.glassfish.kernel.event.EventsImpl.send(EventsImpl.java:128)
	at org.glassfish.internal.data.ApplicationInfo.start(ApplicationInfo.java:298)
	at com.sun.enterprise.v3.server.ApplicationLifecycle.deploy(ApplicationLifecycle.java:461)
	at com.sun.enterprise.v3.server.ApplicationLifecycle.deploy(ApplicationLifecycle.java:240)
	at org.glassfish.deployment.admin.DeployCommand.execute(DeployCommand.java:370)
	at com.sun.enterprise.v3.admin.CommandRunnerImpl$1.execute(CommandRunnerImpl.java:355)
	at com.sun.enterprise.v3.admin.CommandRunnerImpl.doCommand(CommandRunnerImpl.java:370)
	at com.sun.enterprise.v3.admin.CommandRunnerImpl.doCommand(CommandRunnerImpl.java:1067)
	at com.sun.enterprise.v3.admin.CommandRunnerImpl.access$1200(CommandRunnerImpl.java:96)
	at com.sun.enterprise.v3.admin.CommandRunnerImpl$ExecutionContext.execute(CommandRunnerImpl.java:1247)
	at com.sun.enterprise.v3.admin.CommandRunnerImpl$ExecutionContext.execute(CommandRunnerImpl.java:1235)
	at com.sun.enterprise.v3.admin.AdminAdapter.doCommand(AdminAdapter.java:465)
	at com.sun.enterprise.v3.admin.AdminAdapter.service(AdminAdapter.java:222)
	at com.sun.grizzly.tcp.http11.GrizzlyAdapter.service(GrizzlyAdapter.java:168)
	at com.sun.enterprise.v3.server.HK2Dispatcher.dispath(HK2Dispatcher.java:117)
	at com.sun.enterprise.v3.services.impl.ContainerMapper.service(ContainerMapper.java:234)
	at com.sun.grizzly.http.ProcessorTask.invokeAdapter(ProcessorTask.java:822)
	at com.sun.grizzly.http.ProcessorTask.doProcess(ProcessorTask.java:719)
	at com.sun.grizzly.http.ProcessorTask.process(ProcessorTask.java:1013)
	at com.sun.grizzly.http.DefaultProtocolFilter.execute(DefaultProtocolFilter.java:225)
	at com.sun.grizzly.DefaultProtocolChain.executeProtocolFilter(DefaultProtocolChain.java:137)
	at com.sun.grizzly.DefaultProtocolChain.execute(DefaultProtocolChain.java:104)
	at com.sun.grizzly.DefaultProtocolChain.execute(DefaultProtocolChain.java:90)
	at com.sun.grizzly.http.HttpProtocolChain.execute(HttpProtocolChain.java:79)
	at com.sun.grizzly.ProtocolChainContextTask.doCall(ProtocolChainContextTask.java:54)
	at com.sun.grizzly.SelectionKeyContextTask.call(SelectionKeyContextTask.java:59)
	at com.sun.grizzly.ContextTask.run(ContextTask.java:71)
	at com.sun.grizzly.util.AbstractThreadPool$Worker.doWork(AbstractThreadPool.java:532)
	at com.sun.grizzly.util.AbstractThreadPool$Worker.run(AbstractThreadPool.java:513)
	at java.lang.Thread.run(Thread.java:662)