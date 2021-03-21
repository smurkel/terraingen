#pragma once

#include <stdint.h>
#include <queue>
#include <unordered_map>
#include <cassert>
#include "Hazel/Core.h"
#include "Hazel/Log.h"

namespace Hazel
{
	const uint32_t MAX_PROCESSES = 1024;

	class MultiFrameProcess
	{
	public:
		MultiFrameProcess(float* variable, float rateofchange, float targetvalue)
		{
			m_Subject = variable;
			m_Rate = rateofchange;
			m_TargetValue = targetvalue;

			m_OriginalValue = *(m_Subject);
			float _temp = m_TargetValue - m_OriginalValue;
			m_Sign = (_temp > 0) - (_temp < 0);
		}
		bool OnUpdate(float dt)
		{
			*(m_Subject) += m_Sign * m_Rate * dt;
			if (m_Sign * (*(m_Subject)-m_TargetValue) >= 0)
			{
				*(m_Subject) = m_TargetValue;
				return true;
			}
			return false;
		}
		void Undo()
		{
			*(m_Subject) = m_TargetValue;
		}
	private:
		float* m_Subject;
		float m_Rate;
		float m_TargetValue;
		float m_OriginalValue;
		float m_Sign;
	};

	class MFProcessManager // MFP = Multi Frame Process
	{
	public:
		MFProcessManager()
		{
			for (uint32_t mfp = 0; mfp < MAX_PROCESSES; ++mfp)
			{
				mAvailableIDs.push(mfp);
			}
		}
		uint32_t CreateMFProcess(float* variable, float rateofchange, float targetvalue)
		{
			assert(mActiveProcessCount < MAX_PROCESSES, "Maximum number of active multi-frame processes reached.");
			uint32_t mfp = mAvailableIDs.front();
			mAvailableIDs.pop();
			++mActiveProcessCount;
			Ref<MultiFrameProcess> process = std::make_shared<MultiFrameProcess>(variable, rateofchange, targetvalue);
			mActiveProcesses[mfp] = process;
			return mfp;
		}
		void StopMFProcess(uint32_t mfp)
		{
			if (mActiveProcesses.find(mfp) != mActiveProcesses.end())
			{
				mActiveProcesses.erase(mfp);
				mAvailableIDs.push(mfp);
				--mActiveProcessCount;
			}
		}
		void UndoMFProcess(uint32_t mfp)
		{
			if (mActiveProcesses.find(mfp) != mActiveProcesses.end())
			{	
				mActiveProcesses[mfp]->Undo();
				mActiveProcesses.erase(mfp);
				mAvailableIDs.push(mfp);
				--mActiveProcessCount;
			}
		}
		void OnUpdate(float dt)
		{
			std::vector<uint32_t> _deletedKeys;
			for (auto& it: mActiveProcesses)
			{
				if ((it.second)->OnUpdate(dt))
				{
					_deletedKeys.push_back(it.first);
				}
			}
			for (uint32_t key : _deletedKeys)
			{
				StopMFProcess(key);
			}
		}
	private:
		std::queue<uint32_t> mAvailableIDs{};
		std::unordered_map<uint32_t, Ref<MultiFrameProcess>> mActiveProcesses{};
		uint32_t mActiveProcessCount = 0;
	};
}