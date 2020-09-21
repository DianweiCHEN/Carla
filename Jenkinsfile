#!/usr/bin/env groovy

pipeline
{
    agent none

    options
    {
        buildDiscarder(logRotator(numToKeepStr: '3', artifactNumToKeepStr: '3'))
    }

    stages
    {
        stage('Creating nodes')
        {
            agent { label "master" }
            steps
            {
                script
                {
                    JOB_ID = "${env.BUILD_TAG}"
                    jenkinsLib = load("/home/jenkins/jenkins.groovy")
                    
                    jenkinsLib.CreateUbuntuBuildNode(JOB_ID)
                }
            }
        }
        stage('Building CARLA')
        {
            parallel
            {
                stage('ubuntu')
                {
                    agent { label "ubuntu && build && ${JOB_ID}" }
                    environment
                    {
                        UE4_ROOT = '/home/jenkins/UnrealEngine_4.24'
                    }
                    stages
                    {
                        stage('ubuntu retrieve content')
                        {
                            steps
                            {
                                sh './Update.sh'
                            }
                        }
                        stage('ubuntu package')
                        {
                            steps
                            {
                                sh 'make package.rss ARGS="--python-version=2,3.7"'
                            }
                            post 
                            {
                                always 
                                {
                                    archiveArtifacts 'Dist/*.tar.gz'
                                }
                            }
                        }
                    }
                    post 
                    {
                        always 
                        { 
                            deleteDir() 

                            node('master')
                            {
                                script
                                {
                                    JOB_ID = "${env.BUILD_TAG}"
                                    jenkinsLib = load("/home/jenkins/jenkins.groovy")
                                    
                                    jenkinsLib.DeleteUbuntuBuildNode(JOB_ID)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
