# FastStatistics

Fast computation of descriptive statistics in ruby using native code and SIMD.

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

Given you have some 2-dimensional data:
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

You can compute descriptive statistics as follows:

```ruby
FastStatistics::Array2D.new(data).descriptive_statistics
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
