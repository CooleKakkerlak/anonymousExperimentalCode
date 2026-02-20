#include "testFuncs.h"

// #include "windows.h"
// #include "psapi.h"

#include <atomic>
#include <cstdlib>

namespace TestFuncs
{

	// start stop inclusive
	std::vector<int> range(int start, int stop, int stepSize)
	{
		std::vector<int> out;
		for (int x = start; x <= stop; x += stepSize)
		{
			out.push_back(x);
		}
		return out;
	}

	std::vector<Scenario> generateScenarios(std::vector<int> ns, std::vector<int> ss, std::vector<int> ks, std::vector<int> numColorss, double defaultPower, std::vector<double> alphas, std::vector<int> gammas)
	{
		std::vector<Scenario> scenarios;
		for (int n : ns)
			for (int s : ss)
				for (int numColors : numColorss)
					for (double alpha : alphas)
						for (int gamma : gammas)
						{
							Scenario scenario;
							scenario.synthetic = true;
							scenario.numPoints = n;
							if (s == -1)
								scenario.s = (int)std::pow(n, defaultPower);
							else
								scenario.s = s;
							scenario.numColors = numColors;
							scenario.ks = ks;
							scenario.numQueries = 1000;
							scenario.alpha = alpha;
							scenario.gamma = gamma;
							if (!scenario.isValid())
								continue;
							scenarios.push_back(scenario);
						}
		return scenarios;
	}

	std::vector<Scenario> generateRealScenarios(std::string filename, int d0, int d1, std::vector<double> trainFracs, std::vector<int> ks)
	{
		std::vector<Scenario> scenarios;
		for (double trainFrac : trainFracs)
		{
			Scenario scenario;
			scenario.synthetic = false;
			scenario.filename = filename;
			scenario.trainFrac = trainFrac;
			scenario.d0 = d0;
			scenario.d1 = d1;
			scenario.numQueries = 1000;

			scenario.ks = ks;

			scenarios.push_back(scenario);
		}
		return scenarios;
	}

	// each point has color c with probability 1/numColors
	std::vector<Color_> generateUniformColors(const Scenario& scenario, MyRandom& random)
	{
		std::vector<Color_> colors;

		for (int i = 0; i < scenario.numPoints; i++)
		{
			Color_ col = random.nextInt(0, scenario.numColors);
			colors.push_back(col);
		}
		fixColors(scenario, colors, random);
		return colors;
	}

	void fixColors(const Scenario& scenario, std::vector<Color_>& colors, MyRandom& random)
	{
		std::vector<std::vector<int>> indicesPerColor = std::vector<std::vector<int>>(scenario.numColors);
		for (int i = 0; i < scenario.numPoints; i++)
		{
			indicesPerColor[colors[i]].push_back(i);
		}

		// TODO: very messy implementation
		std::vector<Color_> bigColors;
		for (int col = 0; col < scenario.numColors; col++)
		{
			if (indicesPerColor[col].size() > 1)
				bigColors.push_back(col);
		}

		for (int col = 0; col < scenario.numColors; col++)
		{
			if (indicesPerColor[col].size() == 0)
			{
				// color is unused; steal a random point from a random color with more than one point
				int stealColorIndex = random.nextInt(0, bigColors.size());
				Color_ stealColor = bigColors[stealColorIndex];
				int stealIndex = indicesPerColor[stealColor][random.nextInt(0, indicesPerColor[stealColor].size())];
				colors[stealIndex] = col;
				// update indicesPerColor and bigColor
				indicesPerColor[stealColor].erase(std::find(indicesPerColor[stealColor].begin(), indicesPerColor[stealColor].end(), stealIndex)); // linear deletion, kinda slow but doesn't really matter
				if (indicesPerColor[stealColor].size() == 1)
					bigColors.erase(std::find(bigColors.begin(), bigColors.end(), stealColor));
			}
		}

		// make sure it worked
		std::vector<int> colorCounts = std::vector<int>(scenario.numColors);
		for (int i = 0; i < scenario.numPoints; i++)
			colorCounts[colors[i]]++;
		for (int col = 0; col < scenario.numColors; col++)
			if (colorCounts[col] == 0)
				throw std::runtime_error("there is a color with 0 points");
	}

	int countColors(std::vector<Color_>& colors)
	{
		std::set<Color_> uniqueColors;
		Color_ minCol = colors.size(), maxCol = -1;
		for (int i = 0; i < colors.size(); i++)
		{
			minCol = std::min(minCol, colors[i]);
			maxCol = std::max(maxCol, colors[i]);
			uniqueColors.insert(colors[i]);
		}

		if (minCol == 0 && maxCol == uniqueColors.size() - 1)
			return uniqueColors.size();

		std::map<Color_, Color_> colorMapping;
		Color_ cur = 0;
		for (Color_ col : uniqueColors)
		{
			colorMapping[col] = cur++;
		}
		for (int i = 0; i < colors.size(); i++)
		{
			colors[i] = colorMapping[colors[i]];
		}
		// return uniqueColors.size() - 1;
		return countColors(colors); // check again if correct
	}
}