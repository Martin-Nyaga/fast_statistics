require "bundler/gem_tasks"
require "rake/extensiontask"

task :default => :spec

Rake::ExtensionTask.new "fast_statistics" do |ext|
  ext.lib_dir = "lib/fast_statistics"
end
