:
#
# This script checks various configure parameters and uses three files:
#   * autogen.input (ro)
#   * autogen.lastrun (rw)
#   * autogen.lastrun.bak (rw)
#
# If _no_ parameters:
#   Read args from autogen.input or autogen.lastrun
# Else
#   Backup autogen.lastrun as autogen.lastrun.bak
#   Write autogen.lastrun with new commandline args
#
# Run configure with checked args

    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;

use strict;
use Cwd ('cwd', 'realpath');
use File::Basename;

print "S0 = $0 \n";

my $src_path=dirname(realpath($0));
my $build_path=realpath(cwd());

print "src_path = $src_path \n";

chdir ($build_path);

$ENV{PWD} = $build_path;

my $aclocal;
my $autoconf;

printf "aclocal = $aclocal \n";

print "autoconf = $autoconf \n";

# check we have various vital tools
sub sanity_checks($)
{
    print "sanity_checks start\n";
    my $system = shift;
    print "system = $system \n";
    
    my @path = split (':', $ENV{'PATH'});

    #print "path = @path \n";

    my %required =
      (
       'pkg-config' => "pkg-config is required to be installed",
       $autoconf    => "autoconf is required",
       $aclocal     => "$aclocal is required",
      );

    print "in sanity_checks 2 \n";
    for my $elem (@path) {
        for my $app (keys %required) {
            #-f 测试文件为plain file
            if (-f "$elem/$app") {
                print "delete   $required{$app} \n";
                delete $required{$app};
                 
            }
        }
    }
    if ((keys %required) > 0) {
        print ("Various low-level dependencies are missing, please install them:\n");
        for my $app (keys %required) {
            print "\t $app: " . $required{$app} . "\n";
        }
        exit (1);
    }
    print "in sanity_checks end \n";
}

print "after sanity_checks \n";


# one argument per line
sub read_args($)
{
    my $file = shift;
    my $fh;
    my @lst;
    open ($fh, $file) || die "can't open file: $file";
    while (<$fh>) {
        chomp();
        s/^\s+//;
        s/\s+$//;
        # migrate from the old system
        if ( substr($_, 0, 1) eq "'" ) {
            print STDERR "Migrating options from the old autogen.lastrun format, using:\n";
            my @opts;
            @opts = split(/'/);
            foreach my $opt (@opts) {
                if ( substr($opt, 0, 1) eq "-" ) {
                    push @lst, $opt;
                    print STDERR "  $opt\n";
                }
            }
        } elsif ( /^INCLUDE:(.*)/ ) {
            # include another .conf into this one
            my $config = "$src_path/distro-configs/$1.conf";
            if (! -f $config) {
                invalid_distro ($config, $1);
            }
            push @lst, read_args ($config);
        } elsif ( substr($_, 0, 1) eq "#" ) {
            # comment
        } elsif ( length == 0 ) {
            # empty line
        } else {
            push @lst, $_;
        }
    }
    close ($fh);
    # print "read args from file '$file': @lst\n";
    return @lst;
}

print "after read_args \n";

sub show_distro_configs($$)
{
    my ($prefix, $path) = @_;
    my $dirh;
    opendir ($dirh, "$path");
    while (($_ = readdir ($dirh))) {
        if (-d "$path/$_") {
            show_distro_configs(
                    $prefix eq "" ? "$_/" : "$prefix/$_/", "$path/$_")
                unless $_ eq '.' || $_ eq '..';
            next;
        }
        /(.*)\.conf$/ || next;
        print STDERR "\t$prefix$1\n";
    }
    closedir ($dirh);
}

print "after show_distro_configs \n";


sub invalid_distro($$)
{
    my ($config, $distro) = @_;
    print STDERR "Can't find distro option set: $config\n";
    print STDERR "Distros with distro option sets are:\n";
    show_distro_configs("", "$src_path/distro-configs");
    exit (1);
}

print "after invalid_distro \n";

# Avoid confusing "aclocal: error: non-option arguments are not accepted: '.../m4'." error message.
die "\$src_path must not contain spaces, but it is '$src_path'." if ($src_path =~ / /);

print "after die exect \n";


# Alloc $ACLOCAL to specify which aclocal to use
$aclocal = $ENV{ACLOCAL} ? $ENV{ACLOCAL} : 'aclocal';

print "EVN_ACLOCAL = $ENV{ACLOCAL} \n";

# Alloc $AUTOCONF to specify which autoconf to use
# (e.g. autoconf268 from a backports repo)

print "EVN_AUTOCONF1 = $ENV{AUTOCONF} \n";

$autoconf = $ENV{AUTOCONF} ? $ENV{AUTOCONF} : 'autoconf';

print "EVN_AUTOCONF2 = $ENV{AUTOCONF} \n";

#chomp remove the \n at the end of the string
my $system = `uname -s`;
chomp $system;

print "system=$system \n";

sanity_checks ($system) unless($system eq 'Darwin');

print "ENV{LODE_HOME}=$ENV{LODE_HOME} \n";

if (defined $ENV{LODE_HOME})
{   
    print "LODE_HOME start \n";
    my $lode_path = quotemeta "$ENV{LODE_HOME}/opt/bin";
    if($ENV{PATH} !~ $lode_path)
    {
        $ENV{PATH}="$ENV{LODE_HOME}/opt/bin:$ENV{PATH}";
        print STDERR "add LODE_HOME/opt/bin in PATH\n";
    }
    print "LODE_HOME end \n";
}

print "ENV{ACLOCAL_FLAGS} = $ENV{ACLOCAL_FLAGS} \n";
my $aclocal_flags = $ENV{ACLOCAL_FLAGS};

print "aclocal_flags = $aclocal_flags \n";

$aclocal_flags .= " -I $src_path/m4";
$aclocal_flags .= " -I $src_path/m4/mac" if ($system eq 'Darwin');
$aclocal_flags .= " -I /opt/freeware/share/aclocal" if ($system eq 'AIX');

print "aclocal_flags = $aclocal_flags \n";

print "ENV{AUTOMAKE_EXTRA_FLAGS} = $ENV{AUTOMAKE_EXTRA_FLAGS} \n";

$ENV{AUTOMAKE_EXTRA_FLAGS} = '--warnings=no-portability' if (!($system eq 'Darwin'));

print "ENV{AUTOMAKE_EXTRA_FLAGS} = $ENV{AUTOMAKE_EXTRA_FLAGS} \n";

print "src_path = $src_path \n";
print "build_path = $build_path \n";

=pod if ($src_path ne $build_path)
{
    print "src_path ne build_path \n";
    system ("ln -sf $src_path/configure.ac configure.ac");
    system ("ln -sf $src_path/g g");
    my $src_path_win=$src_path;
    if ($system =~ /CYGWIN.*/) {
        $src_path_win=`cygpath -m $src_path`;
        chomp $src_path_win;
        print "src_path_win = $src_path_win in CYGWIN\n";
    }
    my @modules = <$src_path/*/Makefile>;
    
    foreach my $module (@modules)
    {
        #print "modules = $module \n";
        my $dir = basename (dirname ($module));
        #mkdir ($dir);
        #system ("rm -f $dir/Makefile");
        #system ("printf 'module_directory:=$src_path_win/$dir/\ninclude \$(module_directory)/../solenv/gbuild/partial_build.mk\n' > $dir/Makefile");
    }
    my @external_modules = <$src_path/external/*/Makefile>;
    mkdir ("external");
    system ("ln -sf $src_path/external/Module_external.mk external/");
    foreach my $module (@external_modules)
    {
        #print "external modules = $module \n";
        my $dir = basename (dirname ($module));
        #mkdir ("external/$dir");
        #system ("rm -f external/$dir/Makefile");
        system ("printf 'module_directory:=$src_path_win/external/$dir/\ninclude \$(module_directory)/../../solenv/gbuild/partial_build.mk\n' > external/$dir/Makefile");
    }
}
=cut

print "aclocal = $aclocal \n";
print "aclocal_flags = $aclocal_flags \n";

system ("$aclocal $aclocal_flags") && die "Failed to run aclocal";

#remove configure 
unlink ("configure");
# system ("$autoconf -I ${src_path}") && die "Failed to run autoconf";
# die "Failed to generate the configure script" if (! -f "configure");