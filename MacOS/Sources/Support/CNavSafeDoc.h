/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


// Header for CNavSafeDoc class

#ifndef __CNAVSAFEDOCCLICK__MULBERRY__
#define __CNAVSAFEDOCCLICK__MULBERRY__


// Classes

class CNavSafeDoc : public LSingleDoc {

public:

			CNavSafeDoc() {}
			CNavSafeDoc(LCommander* inSuper) : LSingleDoc(inSuper) {}
	virtual ~CNavSafeDoc() {}

	virtual bool AskConfirmRevert();
	
	virtual SInt16 AskSaveChanges(bool inQuitting);
	
};

#endif
