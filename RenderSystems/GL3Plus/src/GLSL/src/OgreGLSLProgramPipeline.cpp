/*
 -----------------------------------------------------------------------------
 This source file is part of OGRE
 (Object-oriented Graphics Rendering Engine)
 For the latest info, see http://www.ogre3d.org/
 
 Copyright (c) 2000-2012 Torus Knot Software Ltd
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
 */

#include "OgreGLSLProgramPipeline.h"
#include "OgreStringConverter.h"
#include "OgreGLSLGpuProgram.h"
#include "OgreGLSLProgram.h"
#include "OgreGLSLProgramPipelineManager.h"
#include "OgreGpuProgramManager.h"
#include "OgreGL3PlusUtil.h"
#include "OgreLogManager.h"

namespace Ogre
{
    GLSLProgramPipeline::GLSLProgramPipeline(GLSLGpuProgram* vertexProgram, GLSLGpuProgram* geometryProgram, GLSLGpuProgram* fragmentProgram, GLSLGpuProgram* hullProgram, GLSLGpuProgram* domainProgram, GLSLGpuProgram* computeProgram) :
    GLSLProgramCommon(vertexProgram, geometryProgram, fragmentProgram, hullProgram, domainProgram, computeProgram) { }

    GLSLProgramPipeline::~GLSLProgramPipeline()
    {
        glDeleteProgramPipelines(1, &mGLProgramPipelineHandle);
        GL_CHECK_ERROR
    }

    void GLSLProgramPipeline::compileAndLink()
	{        
        glGenProgramPipelines(1, &mGLProgramPipelineHandle);
        GL_CHECK_ERROR
        glBindProgramPipeline(mGLProgramPipelineHandle);
        GL_CHECK_ERROR

        mVertexArrayObject = new GL3PlusVertexArrayObject();
        mVertexArrayObject->bind();

        compileIndividualProgram(mVertexProgram);
        compileIndividualProgram(mFragmentProgram);
        compileIndividualProgram(mGeometryProgram);
        compileIndividualProgram(mDomainProgram);
        compileIndividualProgram(mHullProgram);
        compileIndividualProgram(mComputeProgram);

		if(mLinked)
		{
			if ( GpuProgramManager::getSingleton().getSaveMicrocodesToCache() )
			{
				// Add to the microcode to the cache
				String name;
				name = getCombinedName();

				// Get buffer size
				GLint binaryLength = 0;

				glGetProgramiv(mGLProgramPipelineHandle, GL_PROGRAM_BINARY_LENGTH, &binaryLength);
                GL_CHECK_ERROR

                // Create microcode
                GpuProgramManager::Microcode newMicrocode = 
                    GpuProgramManager::getSingleton().createMicrocode((unsigned long)binaryLength + sizeof(GLenum));

				// Get binary
				glGetProgramBinary(mGLProgramPipelineHandle, binaryLength, NULL, (GLenum *)newMicrocode->getPtr(), newMicrocode->getPtr() + sizeof(GLenum));
                GL_CHECK_ERROR

        		// Add to the microcode to the cache
				GpuProgramManager::getSingleton().addMicrocodeToCache(name, newMicrocode);
			}
            if(mVertexProgram && mVertexProgram->isLinked())
            {
                glUseProgramStages(mGLProgramPipelineHandle, GL_VERTEX_SHADER_BIT, mVertexProgram->getGLSLProgram()->getGLProgramHandle());
                GL_CHECK_ERROR
            }
            if(mFragmentProgram && mFragmentProgram->isLinked())
            {
                glUseProgramStages(mGLProgramPipelineHandle, GL_FRAGMENT_SHADER_BIT, mFragmentProgram->getGLSLProgram()->getGLProgramHandle());
                GL_CHECK_ERROR
            }
            if(mGeometryProgram && mGeometryProgram->isLinked())
            {
                glUseProgramStages(mGLProgramPipelineHandle, GL_GEOMETRY_SHADER_BIT, mGeometryProgram->getGLSLProgram()->getGLProgramHandle());
                GL_CHECK_ERROR
            }
            if(mDomainProgram && mDomainProgram->isLinked())
            {
                glUseProgramStages(mGLProgramPipelineHandle, GL_TESS_EVALUATION_SHADER_BIT, mDomainProgram->getGLSLProgram()->getGLProgramHandle());
                GL_CHECK_ERROR
            }
            if(mHullProgram && mHullProgram->isLinked())
            {
                glUseProgramStages(mGLProgramPipelineHandle, GL_TESS_CONTROL_SHADER_BIT, mHullProgram->getGLSLProgram()->getGLProgramHandle());
                GL_CHECK_ERROR
            }
            if(mComputeProgram && mComputeProgram->isLinked())
            {
                glUseProgramStages(mGLProgramPipelineHandle, GL_COMPUTE_SHADER_BIT, mComputeProgram->getGLSLProgram()->getGLProgramHandle());
                GL_CHECK_ERROR
            }

            // Validate pipeline
            logObjectInfo( getCombinedName() + String("GLSL program pipeline result : "), mGLProgramPipelineHandle );

            if(getGLSupport()->checkExtension("GL_KHR_debug") || gl3wIsSupported(4, 3))
                glObjectLabel(GL_PROGRAM_PIPELINE, mGLProgramPipelineHandle, 0,
                                 (mVertexProgram->getName() + "/" + mFragmentProgram->getName()).c_str());
		}
	}

    void GLSLProgramPipeline::compileIndividualProgram(GLSLGpuProgram *program)
    {
        GLint linkStatus = 0;
		// Compile and attach program
        if(program && !program->isLinked())
        {
            try
            {
                program->getGLSLProgram()->compile(true);
            }
            catch (Exception& e)
            {
				LogManager::getSingleton().stream() << e.getDescription();
                mTriedToLinkAndFailed = true;
                return;
            }
            GLuint programHandle = program->getGLSLProgram()->getGLProgramHandle();
            glProgramParameteri(programHandle, GL_PROGRAM_SEPARABLE, GL_TRUE);
            GL_CHECK_ERROR
            program->getGLSLProgram()->attachToProgramObject(programHandle);
            glLinkProgram(programHandle);
            GL_CHECK_ERROR
            glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
            GL_CHECK_ERROR

            program->setLinked(linkStatus);
            mLinked = linkStatus;

            GL_CHECK_ERROR
            mTriedToLinkAndFailed = !linkStatus;

            logObjectInfo( getCombinedName() + String("GLSL program result : "), programHandle );

            if(program->getType() == GPT_VERTEX_PROGRAM)
                setSkeletalAnimationIncluded(program->isSkeletalAnimationIncluded());
        }
    }

    void GLSLProgramPipeline::_useProgram(void)
    {
		if (mLinked)
		{
            glBindProgramPipeline(mGLProgramPipelineHandle);
            GL_CHECK_ERROR
		}
    }

	//-----------------------------------------------------------------------
	GLint GLSLProgramPipeline::getAttributeIndex(VertexElementSemantic semantic, uint index)
	{
		GLint res = mCustomAttributesIndexes[semantic-1][index];
		if (res == NULL_CUSTOM_ATTRIBUTES_INDEX)
		{
            GLuint handle = mVertexProgram->getGLSLProgram()->getGLProgramHandle();
			const char * attString = getAttributeSemanticString(semantic);
			GLint attrib = glGetAttribLocation(handle, attString);
            GL_CHECK_ERROR

			// Sadly position is a special case 
			if (attrib == NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX && semantic == VES_POSITION)
			{
				attrib = glGetAttribLocation(handle, "position");
                GL_CHECK_ERROR
			}
            
			// For uv and other case the index is a part of the name
			if (attrib == NOT_FOUND_CUSTOM_ATTRIBUTES_INDEX)
			{
				String attStringWithSemantic = String(attString) + StringConverter::toString(index);
				attrib = glGetAttribLocation(handle, attStringWithSemantic.c_str());
                GL_CHECK_ERROR
			}
            
			// Update mCustomAttributesIndexes with the index we found (or didn't find) 
			mCustomAttributesIndexes[semantic-1][index] = attrib;
			res = attrib;
		}
        
		return res;
	}

    //-----------------------------------------------------------------------
	void GLSLProgramPipeline::activate(void)
	{
		if (!mLinked && !mTriedToLinkAndFailed)
		{
			GL_CHECK_ERROR
            
			if ( GpuProgramManager::getSingleton().canGetCompiledShaderBuffer() &&
				GpuProgramManager::getSingleton().isMicrocodeAvailableInCache(getCombinedName()) )
			{
				getMicrocodeFromCache();
			}
			else
			{
				compileAndLink();
			}

            extractLayoutQualifiers();

			buildGLUniformReferences();
		}

        _useProgram();
	}

    //-----------------------------------------------------------------------
	void GLSLProgramPipeline::updateUniformBlocks(GpuProgramParametersSharedPtr params,
                                              uint16 mask, GpuProgramType fromProgType)
	{
        // Iterate through the list of uniform buffers and update them as needed
		GLUniformBufferIterator currentBuffer = mGLUniformBufferReferences.begin();
		GLUniformBufferIterator endBuffer = mGLUniformBufferReferences.end();

        const GpuProgramParameters::GpuSharedParamUsageList& sharedParams = params->getSharedParameters();

		GpuProgramParameters::GpuSharedParamUsageList::const_iterator it, end = sharedParams.end();
		for (it = sharedParams.begin(); it != end; ++it)
        {
            for (;currentBuffer != endBuffer; ++currentBuffer)
            {
                GL3PlusHardwareUniformBuffer* hwGlBuffer = static_cast<GL3PlusHardwareUniformBuffer*>(currentBuffer->get());
                GpuSharedParametersPtr paramsPtr = it->getSharedParams();

                // Block name is stored in mSharedParams->mName of GpuSharedParamUsageList items
                GLint UniformTransform = glGetUniformBlockIndex(mGLProgramHandle, it->getName().c_str());
                GL_CHECK_ERROR
                glUniformBlockBinding(mGLProgramHandle, UniformTransform, hwGlBuffer->getGLBufferBinding());
                GL_CHECK_ERROR

                hwGlBuffer->writeData(0, hwGlBuffer->getSizeInBytes(), paramsPtr->getFloatConstantList().data());
            }
        }
	}

    //-----------------------------------------------------------------------
	void GLSLProgramPipeline::buildGLUniformReferences(void)
	{
		if (!mUniformRefsBuilt)
		{
			const GpuConstantDefinitionMap* vertParams = 0;
			const GpuConstantDefinitionMap* fragParams = 0;
			const GpuConstantDefinitionMap* geomParams = 0;
			const GpuConstantDefinitionMap* hullParams = 0;
			const GpuConstantDefinitionMap* domainParams = 0;
			const GpuConstantDefinitionMap* computeParams = 0;
			if (mVertexProgram)
			{
				vertParams = &(mVertexProgram->getGLSLProgram()->getConstantDefinitions().map);
                GLSLProgramPipelineManager::getSingleton().extractUniforms(mVertexProgram->getGLSLProgram()->getGLProgramHandle(),
                                                                           vertParams, NULL, NULL, NULL, NULL, NULL, mGLUniformReferences, mGLUniformBufferReferences);
			}
			if (mGeometryProgram)
			{
				geomParams = &(mGeometryProgram->getGLSLProgram()->getConstantDefinitions().map);
                GLSLProgramPipelineManager::getSingleton().extractUniforms(mGeometryProgram->getGLSLProgram()->getGLProgramHandle(),
                                                                           NULL, geomParams, NULL, NULL, NULL, NULL, mGLUniformReferences, mGLUniformBufferReferences);
			}
			if (mFragmentProgram)
			{
				fragParams = &(mFragmentProgram->getGLSLProgram()->getConstantDefinitions().map);
                GLSLProgramPipelineManager::getSingleton().extractUniforms(mFragmentProgram->getGLSLProgram()->getGLProgramHandle(),
                                                                           NULL, NULL, fragParams, NULL, NULL, NULL, mGLUniformReferences, mGLUniformBufferReferences);
			}
			if (mHullProgram)
			{
				hullParams = &(mHullProgram->getGLSLProgram()->getConstantDefinitions().map);
                GLSLProgramPipelineManager::getSingleton().extractUniforms(mHullProgram->getGLSLProgram()->getGLProgramHandle(),
                                                                           NULL, NULL, NULL, hullParams, NULL, NULL, mGLUniformReferences, mGLUniformBufferReferences);
			}
			if (mDomainProgram)
			{
				domainParams = &(mDomainProgram->getGLSLProgram()->getConstantDefinitions().map);
                GLSLProgramPipelineManager::getSingleton().extractUniforms(mDomainProgram->getGLSLProgram()->getGLProgramHandle(),
                                                                           NULL, NULL, NULL, NULL, domainParams, NULL, mGLUniformReferences, mGLUniformBufferReferences);
			}
			if (mComputeProgram)
			{
				computeParams = &(mComputeProgram->getGLSLProgram()->getConstantDefinitions().map);
                GLSLProgramPipelineManager::getSingleton().extractUniforms(mComputeProgram->getGLSLProgram()->getGLProgramHandle(),
                                                                           NULL, NULL, NULL, NULL, NULL, computeParams, mGLUniformReferences, mGLUniformBufferReferences);
			}

			mUniformRefsBuilt = true;
		}
	}

	//-----------------------------------------------------------------------
	void GLSLProgramPipeline::updateUniforms(GpuProgramParametersSharedPtr params, 
                                           uint16 mask, GpuProgramType fromProgType)
	{
		// Iterate through uniform reference list and update uniform values
		GLUniformReferenceIterator currentUniform = mGLUniformReferences.begin();
		GLUniformReferenceIterator endUniform = mGLUniformReferences.end();

        GLuint progID = 0;
        if(fromProgType == GPT_VERTEX_PROGRAM)
        {
            progID = mVertexProgram->getGLSLProgram()->getGLProgramHandle();
        }
        else if(fromProgType == GPT_FRAGMENT_PROGRAM)
        {
            progID = mFragmentProgram->getGLSLProgram()->getGLProgramHandle();
        }
        else if(fromProgType == GPT_GEOMETRY_PROGRAM)
        {
            progID = mGeometryProgram->getGLSLProgram()->getGLProgramHandle();
        }
        else if(fromProgType == GPT_HULL_PROGRAM)
        {
            progID = mHullProgram->getGLSLProgram()->getGLProgramHandle();
        }
        else if(fromProgType == GPT_DOMAIN_PROGRAM)
        {
            progID = mDomainProgram->getGLSLProgram()->getGLProgramHandle();
        }
        else if(fromProgType == GPT_COMPUTE_PROGRAM)
        {
            progID = mComputeProgram->getGLSLProgram()->getGLProgramHandle();
        }

		for (;currentUniform != endUniform; ++currentUniform)
		{
			// Only pull values from buffer it's supposed to be in (vertex or fragment)
			// This method will be called twice, once for vertex program params, 
			// and once for fragment program params.
			if (fromProgType == currentUniform->mSourceProgType)
			{
				const GpuConstantDefinition* def = currentUniform->mConstantDef;
				if (def->variability & mask)
				{
					GLsizei glArraySize = (GLsizei)def->arraySize;

					// Get the index in the parameter real list
					switch (def->constType)
					{
                        case GCT_FLOAT1:
                            glProgramUniform1fv(progID, currentUniform->mLocation, glArraySize,
                                                params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_FLOAT2:
                            glProgramUniform2fv(progID, currentUniform->mLocation, glArraySize, 
                                                params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_FLOAT3:
                            glProgramUniform3fv(progID, currentUniform->mLocation, glArraySize,
                                                params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_FLOAT4:
                            glProgramUniform4fv(progID, currentUniform->mLocation, glArraySize, 
                                                params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_2X2:
                            glProgramUniformMatrix2fv(progID, currentUniform->mLocation, glArraySize,
                                                      GL_TRUE, params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_3X3:
                            glProgramUniformMatrix3fv(progID, currentUniform->mLocation, glArraySize,
                                                      GL_TRUE, params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_4X4:
                            glProgramUniformMatrix4fv(progID, currentUniform->mLocation, glArraySize, 
                                                      GL_TRUE, params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_INT1:
                            glProgramUniform1iv(progID, currentUniform->mLocation, glArraySize,
                                                params->getIntPointer(def->physicalIndex));
                            break;
                        case GCT_INT2:
                            glProgramUniform2iv(progID, currentUniform->mLocation, glArraySize,
                                                params->getIntPointer(def->physicalIndex));
                            break;
                        case GCT_INT3:
                            glProgramUniform3iv(progID, currentUniform->mLocation, glArraySize,
                                                params->getIntPointer(def->physicalIndex));
                            break;
                        case GCT_INT4:
                            glProgramUniform4iv(progID, currentUniform->mLocation, glArraySize, 
                                                params->getIntPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_2X3:
                            glProgramUniformMatrix2x3fv(progID, currentUniform->mLocation, glArraySize,
                                                        GL_TRUE, params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_2X4:
                            glProgramUniformMatrix2x4fv(progID, currentUniform->mLocation, glArraySize,
                                                        GL_TRUE, params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_3X2:
                            glProgramUniformMatrix3x2fv(progID, currentUniform->mLocation, glArraySize,
                                                        GL_TRUE, params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_3X4:
                            glProgramUniformMatrix3x4fv(progID, currentUniform->mLocation, glArraySize,
                                                        GL_TRUE, params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_4X2:
                            glProgramUniformMatrix4x2fv(progID, currentUniform->mLocation, glArraySize,
                                                        GL_TRUE, params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_4X3:
                            glProgramUniformMatrix4x3fv(progID, currentUniform->mLocation, glArraySize,
                                                        GL_TRUE, params->getFloatPointer(def->physicalIndex));
                            break;
                        case GCT_DOUBLE1:
                            glProgramUniform1dv(progID, currentUniform->mLocation, glArraySize,
                                                params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_DOUBLE2:
                            glProgramUniform2dv(progID, currentUniform->mLocation, glArraySize,
                                                params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_DOUBLE3:
                            glProgramUniform3dv(progID, currentUniform->mLocation, glArraySize,
                                                params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_DOUBLE4:
                            glProgramUniform4dv(progID, currentUniform->mLocation, glArraySize,
                                                params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_DOUBLE_2X2:
                            glProgramUniformMatrix2dv(progID, currentUniform->mLocation, glArraySize,
                                                      GL_TRUE, params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_DOUBLE_3X3:
                            glProgramUniformMatrix3dv(progID, currentUniform->mLocation, glArraySize,
                                                      GL_TRUE, params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_DOUBLE_4X4:
                            glProgramUniformMatrix4dv(progID, currentUniform->mLocation, glArraySize,
                                                      GL_TRUE, params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_DOUBLE_2X3:
                            glProgramUniformMatrix2x3dv(progID, currentUniform->mLocation, glArraySize,
                                                        GL_TRUE, params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_DOUBLE_2X4:
                            glProgramUniformMatrix2x4dv(progID, currentUniform->mLocation, glArraySize,
                                                        GL_TRUE, params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_DOUBLE_3X2:
                            glProgramUniformMatrix3x2dv(progID, currentUniform->mLocation, glArraySize,
                                                        GL_TRUE, params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_DOUBLE_3X4:
                            glProgramUniformMatrix3x4dv(progID, currentUniform->mLocation, glArraySize,
                                                        GL_TRUE, params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_DOUBLE_4X2:
                            glProgramUniformMatrix4x2dv(progID, currentUniform->mLocation, glArraySize,
                                                        GL_TRUE, params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_MATRIX_DOUBLE_4X3:
                            glProgramUniformMatrix4x3dv(progID, currentUniform->mLocation, glArraySize,
                                                        GL_TRUE, params->getDoublePointer(def->physicalIndex));
                            break;
                        case GCT_SAMPLER1D:
                        case GCT_SAMPLER1DSHADOW:
                        case GCT_SAMPLER2D:
                        case GCT_SAMPLER2DSHADOW:
                        case GCT_SAMPLER2DARRAY:
                        case GCT_SAMPLER3D:
                        case GCT_SAMPLERCUBE:
                        case GCT_SAMPLERRECT:
                            // Samplers handled like 1-element ints
                            glProgramUniform1iv(progID, currentUniform->mLocation, 1,
                                                params->getIntPointer(def->physicalIndex));
                            break;
                        case GCT_UNKNOWN:
                        case GCT_SUBROUTINE:
                            break;
                            
					} // End switch
                    GL_CHECK_ERROR
				} // Variability & mask
			} // fromProgType == currentUniform->mSourceProgType
            
  		} // End for
	}
	//-----------------------------------------------------------------------
	void GLSLProgramPipeline::updatePassIterationUniforms(GpuProgramParametersSharedPtr params)
	{
		if (params->hasPassIterationNumber())
		{
			size_t index = params->getPassIterationNumberIndex();
            
			GLUniformReferenceIterator currentUniform = mGLUniformReferences.begin();
			GLUniformReferenceIterator endUniform = mGLUniformReferences.end();
            
			// Need to find the uniform that matches the multi pass entry
			for (;currentUniform != endUniform; ++currentUniform)
			{
				// Get the index in the parameter real list
				if (index == currentUniform->mConstantDef->physicalIndex)
				{
                    GLuint progID = 0;
                    if (mVertexProgram && currentUniform->mSourceProgType == GPT_VERTEX_PROGRAM)
                    {
                        progID = mVertexProgram->getGLSLProgram()->getGLProgramHandle();
                    }

                    if (mFragmentProgram && currentUniform->mSourceProgType == GPT_FRAGMENT_PROGRAM)
                    {
                        progID = mFragmentProgram->getGLSLProgram()->getGLProgramHandle();
                    }

                    if (mGeometryProgram && currentUniform->mSourceProgType == GPT_GEOMETRY_PROGRAM)
                    {
                        progID = mGeometryProgram->getGLSLProgram()->getGLProgramHandle();
                    }

                    if (mDomainProgram && currentUniform->mSourceProgType == GPT_DOMAIN_PROGRAM)
                    {
                        progID = mDomainProgram->getGLSLProgram()->getGLProgramHandle();
                    }

                    if (mHullProgram && currentUniform->mSourceProgType == GPT_HULL_PROGRAM)
                    {
                        progID = mHullProgram->getGLSLProgram()->getGLProgramHandle();
                    }

                    if (mComputeProgram && currentUniform->mSourceProgType == GPT_COMPUTE_PROGRAM)
                    {
                        progID = mComputeProgram->getGLSLProgram()->getGLProgramHandle();
                    }

                    glProgramUniform1fv(progID, currentUniform->mLocation, 1, params->getFloatPointer(index));
                    GL_CHECK_ERROR

                    // There will only be one multipass entry
					return;
				}
			}
		}
    }
}
