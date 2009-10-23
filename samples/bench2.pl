#!/usr/bin/perl

use Getopt::Std;
use Cwd;
use File::Basename;
use File::Find;

###############
my $nusmv= "~/NuSMV/NuSMV-2.4.3-x86_64-linux-gnu/bin/NuSMV";
my $default_checksog="../src/sog-its";
#############

getopt ('mdetbaq');

#parsing the file list description
my $dir = $opt_d or die "Specify target directory -d please! \n$usage" if (!defined $opt_a);

# command line for tool pnddd
my $cwd =  getcwd or die "Can't get cwd, used to decide where to do output.\n";

# the list of all methods (taken from main.cpp)
my @genType = ("SOG","DSOG","SLOG");

my $DEFAULT_TIMEOUT = 120;
# howto use this tool
my $usage="Usage:\n bench.pl -m [generation method list = @genType all] -d [directory base] \n"
  . "example : ./bench.pl -m SOG,DSOG -d test/ \n"
  . "\n other options : \n"
  . " -e pathTochecksogexe (default : currentDir/$default_checksog \n"
  . " -t timeout (default $DEFAULT_TIMEOUT seconds)\n"
  . " -q disable check-sog comparison, simply compute stats for SDD sog-its implem";

my $checksog_exe;
if (defined $opt_e) {
  if ($opt_e !~ /^\//) {
    # not an absolute path
    $checksog_exe = $cwd.'/'.$opt_e;
  } else {
    $checksog_exe = $opt_e;
  }
} else {
  $checksog_exe = $cwd."/".$default_checksog ;
}

if (! -x $checksog_exe  && !defined $opt_a) {
  die "File \'$checksog_exe\' does not seem to refer to the expected checksog executable. Use option -e.\n$usage";
}

my $timeout = defined $opt_t ? $opt_t :  $DEFAULT_TIMEOUT ;

my $doComparison = 1;
if (defined $opt_q) {
  $doComparison = 0;
}

# parsing the methods
my @methods = split /,/,$opt_m or  die "Specify a target method -m please ! \n$usage" if (!defined $opt_a);
my $gens = (join ' ',@genType)."all" ;
foreach my $m (@methods) {
  unless ($gens =~ /$m/) { die "Unknown method $m !\n $usage";}
  if ($m =~ /all/) {
    @methods = @genType;
    last;
  }
}

use strict 'vars';   # After obtaining $opt_xxx, which is not a local variable

my $checksogstate = $cwd."/check-sog" ;

# have to pass context through globs because of File::find syntax restriction
# a glob holding current generation method
my $method ;
# a glob for passing the table under construction
my $globTab ;
## A procedure called on each file of the argument list, fills %table

sub workonfile {
  my $ff = $_ ;
  my @aggregatestats;
  #  Work on all .net source files
  if ( -f $ff && $ff =~/\.net$/  ) {
    print STDERR "work on file $ff\n";

    my $totalticks = 0;
    my $totalstates = 0;
    my $totaltrans = 0;

    my @formulas ;
    my $formff = dirname($ff)."/formula.ltl";
    if ( -r $formff ) {
      push @formulas, $formff;
    }
    $formff = "$ff.ltl";
    if ( -r $formff ) {
      push @formulas, $formff;
    }
    my $nbformula = 0;
    foreach  my $formulaff (@formulas) {
      open IN,$formulaff or die "Bad formula file name";
#      print "Working on formula file : $formulaff \n";
      while (my $line = <IN>) {
	chomp $line;
	$line =~ s/"/\\"/g ;
	$nbformula ++;
#	print "Working on formula : $line \n";
	# ../check-sog -Fformula -c -e invoice.cami.net 1
	my $call = "$checksog_exe -S$method -f\"$line\" -c  $ff|";
	
#	print STDERR $call."\n";
	open MYTOOL,$call;
	my $verdict = 0;
	my $nbstates =0;
	my $nbtrans =0;
	my $ticks = 0;
	my @stats;

	while (my $outline = <MYTOOL>) {
#	  print $outline;
	  if ($outline =~ /(\d+) unique states visited/) {
	    $nbstates = $1;
	  } elsif ($outline =~ /(\d+) ticks for the emptiness/) {
	    $ticks = $1;
	  } elsif ($outline =~ /(\d+) transitions explored/) {
	    $nbtrans = $1;
	  } elsif ($outline =~ /STATS/) {
	    chomp $outline;
	    @stats = split (/\,/,$outline);
	    # drop the first field = "STATS"
	    shift @stats;
	    # remove useless fields in output
	    # the final nbstates
	    shift @stats;
	    # the final SDD sizes
	    splice (@stats,2,2);
	  } elsif ($outline =~ /accepting run exists/ ) {
	    #	    print $outline;
	    $verdict = 1;
	    last;
	  }
	}
	close MYTOOL;

	print "$method, $ff,\"$line\", $nbstates, $ticks, $nbtrans, $verdict, ".(join " , ",@stats)."\n";
      }
    }
    # load result into global results table
#    push @aggregatestats,$totaltrans;
#    push @aggregatestats,$totalticks;
#    push @aggregatestats,$totalstates;
#    $$globTab{$method}{$ff} = \@aggregatestats;

#    print "Totals (SDD/BDD) for $nbformula formula : ticks : $totalticks/$totalticks2 ; States : $totalstates/$totalstates2 ; Trans : $totaltrans/$totaltrans2 \n";
  }
}



sub compute_results {
  $globTab = shift;
  # Call sub workonfile for every file and method specified in arguments (loads %table)
  foreach my $m (@methods) {
    $method = $m;
    print STDERR "Running with method : $method\n";
    find(\&workonfile,$dir);
  }
}

sub print_results {
#  my $outfile = shift ;
  my $tab = shift;
  # Select output file
#  open (OUT,"> $outfile");
#  select OUT;

#  tex_header;
  # Print the resulting %tab
  foreach my $meth (sort (keys %$tab)) {
    my $methPerf = $$tab{$meth};

    print "$meth";
#    print "\\hline\n";
#    print "\\textbf\{$meth\} \\\\ \n";
#    print "\\hline\n";

    foreach my $model (sort(keys %$methPerf)) {
      my $perf = $$methPerf{$model};
      print "$model , ".(join " , ",@$perf). "  \n"; 
    }
  }
#  tex_trailer;

#  select STDOUT;
#  close OUT;
}
#main block


print "Method, Model , Formula, |S|, Ticks, |T|, verdict, Time ,Mem(kb) , peak SDD ,peak DDD ,SDD Hom ,SDD cache ,DDD Hom ,DDD cache,SHom cache \n";

# hash table holding all final results of this run : key=method, value = hash holding model name -> results list
my %table = ();

# do computations
compute_results (\%table);
#print_results ($outff,\%table);
#print_results (\%table);

#for (my $i = 1 ; $i <= $#aggregatestats ; $i++) {
#  print "@aggregatestats[$i],";
#}
print "\n";