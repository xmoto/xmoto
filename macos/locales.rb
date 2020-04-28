#!/usr/bin/ruby
require 'pathname'
require 'pp'

# OS X's preferred locale settings usually yield locales without territories
# (eg: 'fr'), but sometimes with ('zh_TW'). In some cases both options
# exist: 'pt', 'pt_BR' and 'pt_PT'.
#
# X-Moto only has locales with territories, so we need to provide a
# symlink for each non-terr locale. Eg: 'fr' => 'fr_FR'.
# Mostly there's just one territory per locale, so this is simple. But
# sometimes there are multiple options, and we just have to choose.

# These are arbitrarily based on http://smolts.org/static/stats/stats.html
preferred = {
  'pt' => 'BR',
  'ca' => 'ES',
}

dir = Pathname.new(ARGV.shift)
locales = dir.children.map { |x| x.basename.to_s }
#locales << 'ca'

# group by language family
families = {}
locales.each do |loc|
  md = /^([^_]+)(?:_(.*))?/.match(loc)
  fam, terr = *md.captures
  (families[fam] ||= []) << terr
end

# find preferred language for each family
families.each do |fam, locs|
  next if locs.include?(nil) # already have a non-terr locale
  pref = if locs.size == 1
    locs.first
  elsif preferred.include?(fam)
    preferred[fam]
  else
    raise "I need a preference for family #{fam}: #{locs.join(', ')}"
  end
  
  # make a symlink to the preferred language
  full = "#{fam}_#{pref}"
  dir.join(fam).make_symlink(full)
end
