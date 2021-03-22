# frozen_string_literal: true

module Helpers
  def percentile(p, arr, len)
    return arr[len - 1] if p == 100
    rank = p / 100.0 * (len - 1)
    lower = arr[rank.floor]
    upper = arr[rank.floor + 1]
    lower + (upper - lower) * (rank - rank.floor)
  end
end

