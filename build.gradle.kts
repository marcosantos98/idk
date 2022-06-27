import org.jetbrains.kotlin.gradle.tasks.KotlinCompile

plugins {
    kotlin("jvm") version "1.7.0"
    application
}

group = "marco"
version = "1.0-SNAPSHOT"

repositories {
    mavenCentral()
}

val jar by tasks.getting(Jar::class) {
    manifest {
        attributes["Main-Class"] = "marco.Main"
    }
}

dependencies {
    testImplementation(kotlin("test"))
}

tasks.test {
    useJUnitPlatform()
}

tasks.withType<KotlinCompile> {
    kotlinOptions.jvmTarget = "1.8"
}

application {
    mainClass.set("MainKt")
}