#!/usr/bin/env ruby
#encoding: utf-8

def createMatrix(n, m, max, name)
  r = Random.new(280)

  newFile = File.new(name, "a+")

  newFile.write("#{n} #{m}\n")
  n.times do |i|
    m.times do |j|
      newFile.write("#{r.rand(0...max.to_f).round(3)} ")
    end
    newFile.write("\n")
  end
end


createMatrix(ARGV[0].to_i, ARGV[1].to_i, ARGV[2].to_i, ARGV[3])
