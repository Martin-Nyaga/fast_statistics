# frozen_string_literal: true

require 'fast_statistics'

THRESHOLD = 10e-9
describe FastStatistics do
  let(:data) do
    [
      [0.6269, 0.3783, 0.1477, 0.2374],
      [0.4209, 0.1055, 0.8000, 0.2023],
      [0.1124, 0.1021, 0.1936, 0.8566],
      [0.6454, 0.5362, 0.4567, 0.8309]
    ]
  end

  let(:expected_stats) do
    [
      { min: 0.1477, max: 0.6269, mean: 0.347575, variance: 0.032763756875, standard_deviation: 0.1810076155165853 },
      { min: 0.1055, max: 0.8000, mean: 0.382175, variance: 0.071245356875, standard_deviation: 0.2669182587890907 },
      { min: 0.1021, max: 0.8566, mean: 0.316175, variance: 0.098609041875, standard_deviation: 0.3140207666301705 },
      { min: 0.4567, max: 0.8309, mean: 0.617300, variance: 0.019696034999, standard_deviation: 0.1403425630377327 }
    ]
  end

  context "#descriptive_statistics" do
    it "calculates descriptive statistics through SIMD if available" do
      variable_stats = FastStatistics.descriptive_statistics(data)
      variable_stats.each_with_index do |stats, index|
        expected_stats[index].each do |(k, v)|
          expect(stats[k]).to be_within(THRESHOLD).of(v)
        end
      end
    end
  end

  context "#descriptive_statistics_unpacked" do
    it "calculates descriptive statistics with the 'unpacked' algorithm" do
      variable_stats = FastStatistics.descriptive_statistics_unpacked(data)
      variable_stats.each_with_index do |stats, index|
        expected_stats[index].each do |(k, v)|
          expect(stats[k]).to be_within(THRESHOLD).of(v)
        end
      end
    end
  end

  context "simd_enabled?" do
    it "allows to check if simd is enabled" do
      expect { FastStatistics.simd_enabled? }.not_to raise_error
    end
  end
end
