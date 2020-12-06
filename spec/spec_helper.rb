# frozen_string_literal: true

# See http://rubydoc.info/gems/rspec-core/RSpec/Core/Configuration
RSpec.configure do |config|
  config.expect_with :rspec do |expectations|
    expectations.include_chain_clauses_in_custom_matcher_descriptions = true
  end

  config.mock_with :rspec do |mocks|
    mocks.verify_partial_doubles = true
  end

  config.filter_run :focus => true
  config.run_all_when_everything_filtered = true
end

require 'rspec/expectations'
require 'pp'

RSpec::Matchers.define :have_same_statistics_values_as do |expected|
  match do |actual|
    expect(expected.length).to eq(actual.length)

    expected.each_with_index do |stats, index|
      actual[index].each do |(k, v)|
        @index = index
        @key = k
        @expected_value = stats[k]
        @actual_value = v
        expect(v).to be_within(threshold).of(@expected_value)
      end
    end
  end

  def threshold
    @threshold || default_threshold
  end

  def default_threshold
    10e-9
  end

  chain :within_threshold do |threshold|
    @threshold = threshold
  end

  failure_message do |actual|
    <<~MSG
      For #{@key} at index #{@index}
      expected: #{@actual_value} to be within #{threshold} of #{@expected_value}

      expected array:
      #{expected.pretty_inspect}

      actual array:
      #{actual.pretty_inspect}
    MSG
  end
end
