#pragma once
#include "OCLProgramUnit.h"

namespace NeuroflowN
{
	class OCLProgram :
		public OCLProgramUnit
	{
		std::string name;
		cl::Program program;
	public:
		OCLProgram(const OCLIntCtxSPtrT& ctx, const std::string& name);
		cl::Program GetProgram();

	private:
		void Compile();
	};
}