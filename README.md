# FastStatistics
![Build Status](https://travis-ci.com/Martin-Nyaga/fast_statistics.svg?branch=master)

A high performance native ruby extension (written in C++) for computation of
descriptive statistics.

## Overview
This gem provides fast computation of descriptive statistics (min, max,
mean, median, q1, q3, standard_deviation) for a multivariate dataset
(represented as a 2D array) in ruby.

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'fast_statistics'
```

And then execute:

    $ bundle install

Or install it yourself as:

    $ gem install fast_statistics

## Usage

Given you have some multivariate (2-dimensional) data:
```ruby
data = [
  [0.6269, 0.3783, 0.1477, 0.2374],
  [0.4209, 0.1055, 0.8000, 0.2023],
  [0.1124, 0.1021, 0.1936, 0.8566],
  [0.6454, 0.5362, 0.4567, 0.8309],
  [0.4828, 0.1572, 0.5706, 0.4085],
  [0.5594, 0.0979, 0.4078, 0.5885],
  [0.8659, 0.5346, 0.5566, 0.6166],
  [0.7256, 0.5841, 0.8546, 0.3918]
]
```

You can compute descriptive statistics for all the inner arrays as follows:

```ruby
FastStatistics::Array2D.new(data).descriptive_statistics
#=> 
[{:min=>0.1477,
  :max=>0.6269,
  :mean=>0.347575,
  :median=>0.30785,
  :q1=>0.214975,
  :q3=>0.44045,
  :standard_deviation=>0.18100761551658537},
 {:min=>0.1055,
  :max=>0.8,
  :mean=>0.38217500000000004,
  :median=>0.3116,
  :q1=>0.1781,
  :q3=>0.515675,
  :standard_deviation=>0.26691825878909076},
 ...,
 {:min=>0.3918,
  :max=>0.8546,
  :mean=>0.639025,
  :median=>0.6548499999999999,
  :q1=>0.536025,
  :q3=>0.75785,
  :standard_deviation=>0.1718318709523935}]
```

## Implementation Notes

The following factors combined help achieve very high performance:

- It is written in C++ and so can leverage the speed of native execution. 
- It minimises the amount of work done by calculating the statistics in as few
  operations as possible (1 sort + 2 loops). Most native alternatives don't
  provide a built in way to get all these statistics at once, and so typically
  require more loops through the data.
- This gem uses explicit 128-bit-wide SIMD intrinsics (on platforms where they
  are available) to parallelize computations for 2 variables at the
  same time where possible giving an additional speed advantage.

## Benchmarks

The alternatives compared are:
- descriptive_statistics (gem)
- ruby-native-statistics (gem)
- Narray (gem)
- Hand-written ruby (using the same algorithm implemented in C++ in this gem)

You can run the benchmark with `rake benchmark`.

```
Comparing calculated statistics with 10 values for 8 variables...
Test passed, results are equal to 6 decimal places!

Benchmarking with 100,000 values for 12 variables...
Warming up --------------------------------------
   Ruby (desc_stats)     1.000  i/100ms
       Ruby (custom)     1.000  i/100ms
       Ruby (narray)     1.000  i/100ms
 Ruby (native_stats)     1.000  i/100ms
                Fast     3.000  i/100ms
Calculating -------------------------------------
   Ruby (desc_stats)      0.444  (± 0.0%) i/s -      3.000  in   6.764293s
       Ruby (custom)      2.281  (± 0.0%) i/s -     12.000  in   5.262760s
       Ruby (narray)      4.036  (± 0.0%) i/s -     21.000  in   5.210366s
 Ruby (native_stats)      5.791  (± 0.0%) i/s -     29.000  in   5.013041s
                Fast     33.050  (± 3.0%) i/s -    168.000  in   5.088618s

Comparison:
                Fast:       33.0 i/s
 Ruby (native_stats):        5.8 i/s - 5.71x  (± 0.00) slower
       Ruby (narray):        4.0 i/s - 8.19x  (± 0.00) slower
       Ruby (custom):        2.3 i/s - 14.49x  (± 0.00) slower
   Ruby (desc_stats):        0.4 i/s - 74.51x  (± 0.00) slower
```

<!-- TODO: Development
## Development

After checking out the repo, run `bin/setup` to install dependencies. You can
also run `bin/console` for an interactive prompt that will allow you to
experiment.

To install this gem onto your local machine, run `bundle exec rake install`. To
release a new version, update the version number in `version.rb`, and then run
`bundle exec rake release`, which will create a git tag for the version, push
git commits and tags, and push the `.gem` file to
[rubygems.org](https://rubygems.org).
-->

## Contributing

Bug reports and pull requests are welcome on GitHub at
https://github.com/Martin-Nyaga/fast_statistics.

## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).
