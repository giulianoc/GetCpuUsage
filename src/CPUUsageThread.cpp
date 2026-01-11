/*
Copyright (C) Giuliano Catrambone (giulianocatrambone@gmail.com)

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later
 version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Commercial use other than under the terms of the GNU General Public
 License is allowed only after express negotiation of conditions
 with the authors.
*/

#include "CPUUsageThread.h"

#include "GetCpuUsage.h"

#include <deque>
#include <numeric>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ranges.h>


CPUUsageThread::~CPUUsageThread()
{
	if (isRunning())
		stop();
}

void CPUUsageThread::start()
{
	if (_running)
	{
		const std::string errorMessage = "CPUUsageThread already running";
		SPDLOG_ERROR(errorMessage);
		throw std::runtime_error(errorMessage);
	}

	_stopSignal = false;
	_thread = std::thread(&CPUUsageThread::run, this);
	_running = true;
}

void CPUUsageThread::stop()
{
	if (_running)
	{
		_stopSignal = true;
		if (_thread.joinable())
			_thread.join();
	}
	_running = false;
}

bool CPUUsageThread::isRunning() const
{
	return _running;
}

void CPUUsageThread::run()
{
	int cpuStatsUpdateIntervalInSeconds = 10;
	std::chrono::system_clock::time_point lastCPUStats = std::chrono::system_clock::now();

	GetCpuUsage getCpuUsage;

	std::deque<uint16_t> cpuUsageQueue;
	constexpr int numberOfLastCPUUsageToBeChecked = 3;
	for (int cpuUsageIndex = 0; cpuUsageIndex < numberOfLastCPUUsageToBeChecked; cpuUsageIndex++)
		cpuUsageQueue.push_front(0);

	while (!_stopSignal)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

			uint16_t maxCpuUsage = 0;
			uint16_t avgCpuUsage = 0;
			try
			{
				cpuUsageQueue.pop_back();
				cpuUsageQueue.push_front(getCpuUsage.getCpuUsage());

				maxCpuUsage = *std::ranges::max_element(cpuUsageQueue);
				{
					const uint64_t sum = std::accumulate(
						cpuUsageQueue.begin(),
						cpuUsageQueue.end(),
						uint64_t{0}
					);
					avgCpuUsage = static_cast<double>(sum) / cpuUsageQueue.size();
				}

				SPDLOG_INFO(
					"cpuUsageThread"
					", maxCpuUsage: {}"
					", avgCpuUsage: {}"
					", cpuUsageQueue: {}", maxCpuUsage, avgCpuUsage,
					fmt::join(cpuUsageQueue, ", "));

				_cpuUsage.store(maxCpuUsage, std::memory_order_relaxed);
			}
			catch (std::exception &e)
			{
				SPDLOG_ERROR("cpuUsage thread failed"
					", e.what(): {}", e.what());
			}

			if (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - lastCPUStats).count()
				>= cpuStatsUpdateIntervalInSeconds)
			{
				lastCPUStats = std::chrono::system_clock::now();
				try
				{
					newCPUUsageAvailable(maxCpuUsage);
				}
				catch (std::exception &e)
				{
					SPDLOG_ERROR("newCPUUsageAvailable failed"
						", exception: {}",
						e.what()
					);
				}
			}
		}
		catch (std::exception &e)
		{
			SPDLOG_ERROR(
				"System::getCPUUsage failed"
				", exception: {}",
				e.what()
			);
		}
	}
}

uint16_t CPUUsageThread::getCPUUsage() const {
	return _cpuUsage.load(std::memory_order_relaxed);
};

void CPUUsageThread::newCPUUsageAvailable(uint16_t& cpuUsage) const
{
	// default implementation does nothing
}
