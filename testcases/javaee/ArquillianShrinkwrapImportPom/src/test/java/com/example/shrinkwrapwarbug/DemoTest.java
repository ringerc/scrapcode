package com.example.shrinkwrapwarbug;

import org.jboss.arquillian.container.test.api.Deployment;
import org.jboss.arquillian.junit.Arquillian;
import org.jboss.shrinkwrap.api.ShrinkWrap;
import org.jboss.shrinkwrap.api.spec.WebArchive;
import org.jboss.shrinkwrap.resolver.api.DependencyResolvers;
import org.jboss.shrinkwrap.resolver.api.maven.MavenDependencyResolver;
import org.junit.*;
import static org.junit.Assert.*;
import org.junit.runner.RunWith;

@RunWith(Arquillian.class)
public class DemoTest {

    @Deployment
    public static WebArchive createDeployment() {
        
        MavenDependencyResolver resolver = DependencyResolvers.use(MavenDependencyResolver.class);
        resolver.configureFrom("pom.xml");
        
        // If we let the MavenDependencyResolver pull the artifact in with the version
        // from the seam 3 bom using the co-ordinates:
        //
        //String SEAM_SECURITY_ARTIFACT = "org.jboss.seam.security:seam-security";
        //
        // it fails with:
        //   java.lang.IllegalArgumentException: Bad artifact coordinates, expected format is <groupId>:<artifactId>[:<extension>[:<classifier>]]:<version>
        //
        // If we instead specify a valid version known to the pom.xml
        // like:
        String SEAM_SECURITY_ARTIFACT = "org.jboss.seam.security:seam-security:3.1.0.Final";
        // 
        // then it instead fails with:
        //
        //   org.sonatype.aether.transfer.ArtifactNotFoundException: Could not find artifact org.jboss.seam.security:seam-security:jar:3.1.0.Final in central (http://repo1.maven.org/maven2)
        //
        // even though the artifact is in the local repository and in a repo referenced from the
        // pom we imported.
        //
        
        return ShrinkWrap.create(WebArchive.class, "demo.war")
                .addClass(Demo.class)
                .addAsLibraries(resolver.artifacts(
                    SEAM_SECURITY_ARTIFACT
                ).resolveAsFiles());
    }
    
    @Test
    public void testReturnOne() {
        Demo d = new Demo();
        assertEquals(d.returnOne(), 1);
    }
    
}
