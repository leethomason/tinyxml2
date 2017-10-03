Pod::Spec.new do |s|
  s.name             = 'tinyxml'
  s.version          = '5.1.0'
  s.summary          = "Tinyxml parser" 

  s.description      = <<-DESC
TODO: Add long description of the pod here.
                       DESC
  s.homepage         = "git@github.com:ViewInc/tinyxml2.git"
  s.license          = { :type => 'MIT', :file => 'LICENSE' }
  s.author           = { 'papiguy' => 'nitin.khanna@viewglass.com' }
  s.source           = { :git => 'git@github.com:ViewInc/tinyxml2.git', :tag => s.version.to_s }

  s.ios.deployment_target = '9.3'
  s.source_files = 'tinyxml2.{cpp,h}'
  s.public_header_files = 'tinyxml2.h'
  s.requires_arc = false
  s.ios.xcconfig = { 
        'GCC_C_LANGUAGE_STANDARD' => 'c11'
        }
end
