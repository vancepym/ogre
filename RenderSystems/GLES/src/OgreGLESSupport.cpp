/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2008 Renato Araujo Oliveira Filho <renatox@gmail.com>
Copyright (c) 2000-2009 Torus Knot Software Ltd

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

#include "OgreGLESSupport.h"

#include "OgreLogManager.h"
#include "OgreException.h"

namespace Ogre {
    void GLESSupport::setConfigOption(const String &name, const String &value)
    {
        ConfigOptionMap::iterator it = mOptions.find(name);

        if (it == mOptions.end())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Option named " + name +  " does not exist.",
                        "GLESSupport::setConfigOption");
        }
        else
        {
            it->second.currentValue = value;
        }
    }

    ConfigOptionMap& GLESSupport::getConfigOptions(void)
    {
        return mOptions;
    }

    void GLESSupport::initialiseExtensions(void)
    {
        // Set version string
        const GLubyte* pcVer = glGetString(GL_VERSION);

        assert(pcVer && "Problems getting GL version string using glGetString");

        String tmpStr = (const char*)pcVer;
        LogManager::getSingleton().logMessage("GL_VERSION = " + tmpStr);
        mVersion = tmpStr.substr(0, tmpStr.find(" "));

        // Get vendor
        const GLubyte* pcVendor = glGetString(GL_VENDOR);
        tmpStr = (const char*)pcVendor;
        LogManager::getSingleton().logMessage("GL_VENDOR = " + tmpStr);
        mVendor = tmpStr.substr(0, tmpStr.find(" "));

        // Get renderer
        const GLubyte* pcRenderer = glGetString(GL_RENDERER);
        tmpStr = (const char*)pcRenderer;
        LogManager::getSingleton().logMessage("GL_RENDERER = " + tmpStr);

        // Set extension list
        std::stringstream ext;
        String str;

        const GLubyte* pcExt = glGetString(GL_EXTENSIONS);
        LogManager::getSingleton().logMessage("GL_EXTENSIONS = " + String((const char*)pcExt));

        assert(pcExt && "Problems getting GL extension string using glGetString");

        ext << pcExt;

        while (ext >> str)
        {
            printf("EXT: %s\n", str.c_str());
            extensionList.insert(str);
        }
        
        // Get function pointers on non-iPhone platforms
#if OGRE_PLATFORM != OGRE_PLATFORM_IPHONE
        glIsRenderbufferOES = (PFNGLISRENDERBUFFEROES)getProcAddress("glIsRenderbufferOES");
        glBindRenderbufferOES = (PFNGLBINDRENDERBUFFEROES)getProcAddress("glBindRenderbufferOES");
        glDeleteRenderbuffersOES = (PFNGLDELETERENDERBUFFERSOES)getProcAddress("glDeleteRenderbuffersOES");
        glGenRenderbuffersOES = (PFNGLGENRENDERBUFFERSOES)getProcAddress("glGenRenderbuffersOES");
        glRenderbufferStorageOES = (PFNGLRENDERBUFFERSTORAGEOES)getProcAddress("glRenderbufferStorageOES");
        glGetRenderbufferParameterivOES = (PFNGLGETRENDERBUFFERPARAMETERIVOES)getProcAddress("glGetRenderbufferParameterivOES");
        glIsFramebufferOES = (PFNGLISFRAMEBUFFEROES)getProcAddress("glIsFramebufferOES");
        glBindFramebufferOES = (PFNGLBINDFRAMEBUFFEROES)getProcAddress("glBindFramebufferOES");
        glDeleteFramebuffersOES = (PFNGLDELETEFRAMEBUFFERSOES)getProcAddress("glDeleteFramebuffersOES");
        glGenFramebuffersOES = (PFNGLGENFRAMEBUFFERSOES)getProcAddress("glGenFramebuffersOES");
        glCheckFramebufferStatusOES = (PFNGLCHECKFRAMEBUFFERSTATUSOES)getProcAddress("glCheckFramebufferStatusOES");
        glFramebufferRenderbufferOES = (PFNGLFRAMEBUFFERRENDERBUFFEROES)getProcAddress("glFramebufferRenderbufferOES");
        glFramebufferTexture2DOES = (PFNGLFRAMEBUFFERTEXTURE2DOES)getProcAddress("glFramebufferTexture2DOES");
        glGetFramebufferAttachmentParameterivOES = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVOES)getProcAddress("glGetFramebufferAttachmentParameterivOES");
        glGenerateMipmapOES = (PFNGLGENERATEMIPMAPOES)getProcAddress("glGenerateMipmapOES");
#endif
    }

    bool GLESSupport::checkExtension(const String& ext) const
    {
        if(extensionList.find(ext) == extensionList.end())
            return false;

        return true;
    }
}