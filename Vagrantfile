# -*- mode: ruby -*-
# vi: set ft=ruby :

# All Vagrant configuration is done below. The "2" in Vagrant.configure
# configures the configuration version (we support older styles for
# backwards compatibility). Please don't change it unless you know what
# you're doing.
Vagrant.configure(2) do |config|
  # The most common configuration options are documented and commented below.
  # For a complete reference, please see the online documentation at
  # https://docs.vagrantup.com.

  # Every Vagrant development environment requires a box. You can search for
  # boxes at https://atlas.hashicorp.com/search.
  config.vm.box = "ubuntu/trusty64"

  # Add another path to the current directory at ~lab2
  config.vm.synced_folder ".", "/home/vagrant/skvlr"

  # Give the virtual machine a little OOMPH. Sure Sergio. Let's go with that.
  config.vm.provider "virtualbox" do |v|
    v.memory = 4096
    v.cpus = 4
    v.customize ["guestproperty", "set", :id,
        "/VirtualBox/GuestAdd/VBoxService/--timesync-set-threshold", 1000]
    v.customize ['guestproperty', 'set', :id,
        "/VirtualBox/GuestAdd/VBoxService/--timesync-interval", 1000]
  end

  # Install pkg-config and the fuse headers
  config.vm.provision "shell", inline: <<-SHELL
    sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
    sudo apt-get update
    sudo apt-get install -y pkg-config
    sudo apt-get install -y automake
    sudo apt-get install -y g++-4.9
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.9 40

  SHELL
end
