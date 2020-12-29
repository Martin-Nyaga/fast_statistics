# frozen_string_literal: true

require 'fast_statistics'

describe FastStatistics::Array2D do
  let(:data) do
    [
      [0.6269, 0.3783, 0.1477, 0.2374],
      [0.4209, 0.1055, 0.8000, 0.2023],
      [0.1124, 0.1021, 0.1936, 0.8566],
      [0.6454, 0.5362, 0.4567, 0.8309],
      [0.4828, 0.1572, 0.5706, 0.4085],
      [0.5594, 0.0979, 0.4078, 0.5885],
      [0.8659, 0.5346, 0.5566, 0.6166],
      [0.7256, 0.5841, 0.8546, 0.3918]
    ]
  end

  let(:expected_stats) do
    [
      { min: 0.1477, max: 0.6269, mean: 0.347575, median: 0.30785, q1: 0.214975, q3: 0.440450, standard_deviation: 0.1810076155165853 },
      { min: 0.1055, max: 0.8000, mean: 0.382175, median: 0.31160, q1: 0.178100, q3: 0.515675, standard_deviation: 0.2669182587890907 },
      { min: 0.1021, max: 0.8566, mean: 0.316175, median: 0.15300, q1: 0.109825, q3: 0.359350, standard_deviation: 0.3140207666301705 },
      { min: 0.4567, max: 0.8309, mean: 0.617300, median: 0.59080, q1: 0.516325, q3: 0.691775, standard_deviation: 0.1403425630377327 },
      { min: 0.1572, max: 0.5706, mean: 0.404775, median: 0.44565, q1: 0.345675, q3: 0.504750, standard_deviation: 0.1540236081742016 },
      { min: 0.0979, max: 0.5885, mean: 0.413400, median: 0.48360, q1: 0.330325, q3: 0.566675, standard_deviation: 0.1946455881852964 },
      { min: 0.5346, max: 0.8659, mean: 0.643425, median: 0.58660, q1: 0.551100, q3: 0.678925, standard_deviation: 0.1319054277692923 },
      { min: 0.3918, max: 0.8546, mean: 0.639025, median: 0.65485, q1: 0.536025, q3: 0.757850, standard_deviation: 0.1718318709523935 }
    ]
  end

  context "with default initialization (double, packed)" do
    subject { FastStatistics::Array2D.new(data) }

    it "#descriptive_statistics works" do
      stats = subject.descriptive_statistics
      expect(stats).to have_same_statistics_values_as(expected_stats)
    end
  end

  context "with explicit initialization (dtype: double, packed: true)" do
    subject { FastStatistics::Array2D.new(data, dtype: :double, packed: true) }

    it "#descriptive_statistics works" do
      stats = subject.descriptive_statistics
      expect(stats).to have_same_statistics_values_as(expected_stats)
    end

    context "with 1 variable" do
      subject { FastStatistics::Array2D.new(data.first(1), dtype: :double, packed: true) }
      it "#descriptive_statistics works" do
        stats = subject.descriptive_statistics
        expect(stats).to have_same_statistics_values_as(expected_stats.first(1))
      end
    end

    context "with an odd number of variables" do
      subject { FastStatistics::Array2D.new(data.first(5), dtype: :double, packed: true) }
      it "#descriptive_statistics works" do
        stats = subject.descriptive_statistics
        expect(stats).to have_same_statistics_values_as(expected_stats.first(5))
      end
    end
  end

  context "with explicit initialization (dtype: float, packed: true)" do
    subject { FastStatistics::Array2D.new(data, dtype: :float, packed: true) }
    let(:threshold) { 10e-6 }

    it "#descriptive_statistics works" do
      stats = subject.descriptive_statistics
      expect(stats).to have_same_statistics_values_as(expected_stats).within_threshold(threshold)
    end

    context "with 1 variable" do
      subject { FastStatistics::Array2D.new(data.first(1), dtype: :float, packed: true) }
      it "#descriptive_statistics works" do
        stats = subject.descriptive_statistics
        expect(stats).to have_same_statistics_values_as(expected_stats.first(1)).within_threshold(threshold)
      end
    end

    context "with an odd number of variables" do
      subject { FastStatistics::Array2D.new(data.first(5), dtype: :float, packed: true) }
      it "#descriptive_statistics works" do
        stats = subject.descriptive_statistics
        expect(stats).to have_same_statistics_values_as(expected_stats.first(5)).within_threshold(threshold)
      end
    end
  end

  context "with explicit initialization (dtype: double, packed: false)" do
    subject { FastStatistics::Array2D.new(data, dtype: :double, packed: false) }

    it "#descriptive_statistics works" do
      stats = subject.descriptive_statistics
      expect(stats).to have_same_statistics_values_as(expected_stats)
    end
  end

  context "with explicit initialization (dtype: float, packed: false)" do
    subject { FastStatistics::Array2D.new(data, dtype: :float, packed: false) }
    let(:threshold) { 10e-6 }

    it "#descriptive_statistics works" do
      stats = subject.descriptive_statistics
      expect(stats).to have_same_statistics_values_as(expected_stats).within_threshold(threshold)
    end
  end

  context "simd_enabled?" do
    it "allows to check if simd is enabled" do
      expect { FastStatistics.simd_enabled? }.not_to raise_error
    end
  end
end
