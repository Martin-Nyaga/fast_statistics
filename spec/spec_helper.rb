# frozen_string_literal: true

# See http://rubydoc.info/gems/rspec-core/RSpec/Core/Configuration
RSpec.configure do |config|
  config.expect_with :rspec do |expectations|
    # This option will default to `true` in RSpec 4. It makes the `description`
    # and `failure_message` of custom matchers include text for helper methods
    # defined using `chain`, e.g.:
    #     be_bigger_than(2).and_smaller_than(4).description
    #     # => "be bigger than 2 and smaller than 4"
    # ...rather than:
    #     # => "be bigger than 2"
    expectations.include_chain_clauses_in_custom_matcher_descriptions = true
  end

  config.mock_with :rspec do |mocks|
    # Prevents you from mocking or stubbing a method that does not exist on
    # a real object. This is generally recommended, and will default to
    # `true` in RSpec 4.
    mocks.verify_partial_doubles = true
  end
end

require 'rspec/expectations'
require 'pp'

RSpec::Matchers.define :have_same_statistics_values_as do |expected|
  match do |actual|
    actual.each_with_index do |stats, index|
      expected[index].each do |(k, v)|
        expect(stats[k]).to be_within(threshold).of(v)
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
      expected
      #{expected.pretty_inspect}
      to have same statistics values as
      #{actual.pretty_inspect}
      within #{threshold}
    MSG
  end
end
