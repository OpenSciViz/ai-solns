#!/usr/bin/env perl
package AH::UTRCopy;
use strict;
use warnings;
use diagnostics;

our (@ISA, @EXPORT_OK);
BEGIN {
  # Keep older versions of Perl from trying to use lexical warnings
  $INC{'warnings.pm'} = "fake warnings entry for < 5.6 perl ($])" if $] < 5.006;
  require Exporter;
  @ISA = qw(Exporter);
  # symbols to export on request
  @EXPORT_OK = qw(fcopy rcopy dircopy fmove rmove dirmove pathmk pathrm pathempty pathrmdir);
}

use Carp;
use File::Copy; 
use File::Spec; #not really needed because File::Copy already gets it, but for good measure :)

use vars qw( 
    $MaxDepth $KeepMode $CPRFComp $CopyLink 
    $PFSCheck $RemvBase $NoFtlPth  $ForcePth $CopyLoop $RMTrgFil $RMTrgDir 
    $CondCopy $BdTrgWrn $SkipFlop  $DirPerms
);

our $VERSION = '0.38';

our $MaxDepth = 0;
our $KeepMode = 1;
our $CPRFComp = 0; 
our $CopyLink = eval { local $SIG{'__DIE__'}; symlink '',''; 1 } || 0;
our $PFSCheck = 1;
our $RemvBase = 0;
our $NoFtlPth = 0;
our $ForcePth = 0;
our $CopyLoop = 0;
our $RMTrgFil = 0;
our $RMTrgDir = 0;
our $CondCopy = {};
our $BdTrgWrn = 0;
our $SkipFlop = 0;
our $DirPerms = 0777; 

my $samecheck = sub {
   return 1 if $^O eq 'MSWin32'; # need better way to check for this on winders...
   return if @_ != 2 || !defined $_[0] || !defined $_[1];
   return if $_[0] eq $_[1];

   my $one = '';
   if($PFSCheck) {
      $one    = join( '-', ( stat $_[0] )[0,1] ) || '';
      my $two = join( '-', ( stat $_[1] )[0,1] ) || '';
      if ( $one eq $two && $one ) {
          carp "$_[0] and $_[1] are identical";
          return;
      }
   }

   if(-d $_[0] && !$CopyLoop) {
      $one    = join( '-', ( stat $_[0] )[0,1] ) if !$one;
      my $abs = File::Spec->rel2abs($_[1]);
      my @pth = File::Spec->splitdir( $abs );
      while(@pth) {
         my $cur = File::Spec->catdir(@pth);
         last if !$cur; # probably not necessary, but nice to have just in case :)
         my $two = join( '-', ( stat $cur )[0,1] ) || '';
         if ( $one eq $two && $one ) {
             # $! = 62; # Too many levels of symbolic links
             carp "Caught Deep Recursion Condition: $_[0] contains $_[1]";
             return;
         }
      
         pop @pth;
      }
   }

   return 1;
};

my $glob = sub {
    my ($do, $src_glob, @args) = @_;
    
    local $CPRFComp = 1;
    
    my @rt;
    for my $path ( glob($src_glob) ) {
        my @call = [$do->($path, @args)] or return;
        push @rt, \@call;
    }
    
    return @rt;
};

my $move = sub {
   my $fl = shift;
   my @x;
   if($fl) {
      @x = fcopy(@_) or return;
   } else {
      @x = dircopy(@_) or return;
   }
   if(@x) {
      if($fl) {
         unlink $_[0] or return;
      } else {
         pathrmdir($_[0]) or return;
      }
      if($RemvBase) {
         my ($volm, $path) = File::Spec->splitpath($_[0]);
         pathrm(File::Spec->catpath($volm,$path,''), $ForcePth, $NoFtlPth) or return;
      }
   }
  return wantarray ? @x : $x[0];
};

my $ok_todo_asper_condcopy = sub {
    my $org = shift;
    my $copy = 1;
    if(exists $CondCopy->{$org}) {
        if($CondCopy->{$org}{'md5'}) {

        }
        if($copy) {

        }
    }
    return $copy;
};

sub fcopy { 
   $samecheck->(@_) or return;
   if($RMTrgFil && (-d $_[1] || -e $_[1]) ) {
      my $trg = $_[1];
      if( -d $trg ) {
        my @trgx = File::Spec->splitpath( $_[0] );
        $trg = File::Spec->catfile( $_[1], $trgx[ $#trgx ] );
      }
      $samecheck->($_[0], $trg) or return;
      if(-e $trg) {
         if($RMTrgFil == 1) {
            unlink $trg or carp "\$RMTrgFil failed: $!";
         } else {
            unlink $trg or return;
         }
      }
   }
   my ($volm, $path) = File::Spec->splitpath($_[1]);
   if($path && !-d $path) {
      pathmk(File::Spec->catpath($volm,$path,''), $NoFtlPth);
   }
   if( -l $_[0] && $CopyLink ) {
      carp "Copying a symlink ($_[0]) whose target does not exist" 
          if !-e readlink($_[0]) && $BdTrgWrn;
      symlink readlink(shift()), shift() or return;
   } else {  
      copy(@_) or return;

      my @base_file = File::Spec->splitpath($_[0]);
      my $mode_trg = -d $_[1] ? File::Spec->catfile($_[1], $base_file[ $#base_file ]) : $_[1];

      chmod scalar((stat($_[0]))[2]), $mode_trg if $KeepMode;
   }
   return wantarray ? (1,0,0) : 1; # use 0's incase they do math on them and in case rcopy() is called in list context = no uninit val warnings
}

sub rcopy { 
    if (-l $_[0] && $CopyLink) {
        goto &fcopy;    
    }
    
    goto &dircopy if -d $_[0] || substr( $_[0], ( 1 * -1), 1) eq '*';
    goto &fcopy;
}

sub rcopy_glob {
    $glob->(\&rcopy, @_);
}

sub dircopy {
   if($RMTrgDir && -d $_[1]) {
      if($RMTrgDir == 1) {
         pathrmdir($_[1]) or carp "\$RMTrgDir failed: $!";
      } else {
         pathrmdir($_[1]) or return;
      }
   }
   my $globstar = 0;
   my $_zero = $_[0];
   my $_one = $_[1];
   if ( substr( $_zero, ( 1 * -1 ), 1 ) eq '*') {
       $globstar = 1;
       $_zero = substr( $_zero, 0, ( length( $_zero ) - 1 ) );
   }

   $samecheck->(  $_zero, $_[1] ) or return;
   if ( !-d $_zero || ( -e $_[1] && !-d $_[1] ) ) {
       $! = 20; 
       return;
   } 

   if(!-d $_[1]) {
      pathmk($_[1], $NoFtlPth) or return;
   } else {
      if($CPRFComp && !$globstar) {
         my @parts = File::Spec->splitdir($_zero);
         while($parts[ $#parts ] eq '') { pop @parts; }
         $_one = File::Spec->catdir($_[1], $parts[$#parts]);
      }
   }
   my $baseend = $_one;
   my $level   = 0;
   my $filen   = 0;
   my $dirn    = 0;

   my $recurs; #must be my()ed before sub {} since it calls itself
   $recurs =  sub {
      my ($str,$end,$buf) = @_;
      $filen++ if $end eq $baseend; 
      $dirn++ if $end eq $baseend;
      
      $DirPerms = oct($DirPerms) if substr($DirPerms,0,1) eq '0';
      mkdir($end,$DirPerms) or return if !-d $end;
      chmod scalar((stat($str))[2]), $end if $KeepMode;
      if($MaxDepth && $MaxDepth =~ m/^\d+$/ && $level >= $MaxDepth) {
         return ($filen,$dirn,$level) if wantarray;
         return $filen;
      }
      $level++;

      
      my @files;
      if ( $] < 5.006 ) {
          opendir(STR_DH, $str) or return;
          @files = grep( $_ ne '.' && $_ ne '..', readdir(STR_DH));
          closedir STR_DH;
      }
      else {
          opendir(my $str_dh, $str) or return;
          @files = grep( $_ ne '.' && $_ ne '..', readdir($str_dh));
          closedir $str_dh;
      }

      for my $file (@files) {
          my ($file_ut) = $file =~ m{ (.*) }xms;
          my $org = File::Spec->catfile($str, $file_ut);
          my $new = File::Spec->catfile($end, $file_ut);
          if( -l $org && $CopyLink ) {
              carp "Copying a symlink ($org) whose target does not exist" 
                  if !-e readlink($org) && $BdTrgWrn;
              symlink readlink($org), $new or return;
          } 
          elsif(-d $org) {
              $recurs->($org,$new,$buf) if defined $buf;
              $recurs->($org,$new) if !defined $buf;
              $filen++;
              $dirn++;
          } 
          else {
              if($ok_todo_asper_condcopy->($org)) {
                  if($SkipFlop) {
                      fcopy($org,$new,$buf) or next if defined $buf;
                      fcopy($org,$new) or next if !defined $buf;                      
                  }
                  else {
                      fcopy($org,$new,$buf) or return if defined $buf;
                      fcopy($org,$new) or return if !defined $buf;
                  }
                  chmod scalar((stat($org))[2]), $new if $KeepMode;
                  $filen++;
              }
          }
      }
      1;
   };

   $recurs->($_zero, $_one, $_[2]) or return;
   return wantarray ? ($filen,$dirn,$level) : $filen;
}

sub fmove { $move->(1, @_) } 

sub rmove { 
    if (-l $_[0] && $CopyLink) {
        goto &fmove;    
    }
    
    goto &dirmove if -d $_[0] || substr( $_[0], ( 1 * -1), 1) eq '*';
    goto &fmove;
}

sub rmove_glob {
    $glob->(\&rmove, @_);
}

sub dirmove { $move->(0, @_) }

sub pathmk {
   my @parts = File::Spec->splitdir( shift() );
   my $nofatal = shift;
   my $pth = $parts[0];
   my $zer = 0;
   if(!$pth) {
      $pth = File::Spec->catdir($parts[0],$parts[1]);
      $zer = 1;
   }
   for($zer..$#parts) {
      $DirPerms = oct($DirPerms) if substr($DirPerms,0,1) eq '0';
      mkdir($pth,$DirPerms) or return if !-d $pth && !$nofatal;
      mkdir($pth,$DirPerms) if !-d $pth && $nofatal;
      $pth = File::Spec->catdir($pth, $parts[$_ + 1]) unless $_ == $#parts;
   }
   1;
} 

sub pathempty {
   my $pth = shift; 

   return 2 if !-d $pth;

   my @names;
   my $pth_dh;
   if ( $] < 5.006 ) {
       opendir(PTH_DH, $pth) or return;
       @names = grep !/^\.+$/, readdir(PTH_DH);
   }
   else {
       opendir($pth_dh, $pth) or return;
       @names = grep !/^\.+$/, readdir($pth_dh);       
   }
   
   for my $name (@names) {
      my ($name_ut) = $name =~ m{ (.*) }xms;
      my $flpth     = File::Spec->catdir($pth, $name_ut);

      if( -l $flpth ) {
	      unlink $flpth or return; 
      }
      elsif(-d $flpth) {
          pathrmdir($flpth) or return;
      } 
      else {
          unlink $flpth or return;
      }
   }

   if ( $] < 5.006 ) {
       closedir PTH_DH;
   }
   else {
       closedir $pth_dh;
   }
   
   1;
}

sub pathrm {
   my $path = shift;
   return 2 if !-d $path;
   my @pth = File::Spec->splitdir( $path );
   my $force = shift;

   while(@pth) { 
      my $cur = File::Spec->catdir(@pth);
      last if !$cur; # necessary ??? 
      if(!shift()) {
         pathempty($cur) or return if $force;
         rmdir $cur or return;
      } 
      else {
         pathempty($cur) if $force;
         rmdir $cur;
      }
      pop @pth;
   }
   1;
}

sub pathrmdir {
    my $dir = shift;
    if( -e $dir ) {
        return if !-d $dir;
    }
    else {
        return 2;
    }

    pathempty($dir) or return;
    
    rmdir $dir or return;
}

1;

__END__

=head1 NAME

AH::UTRCopy - Perl extension for recursively copying files and directories. 
              This is repackaged from CPAN's File::Copy::Recursive version 0.38. 

=head1 SYNOPSIS

  use AH::UTRCopy qw(fcopy rcopy dircopy fmove rmove dirmove);

  fcopy($orig,$new[,$buf]) or die $!;
  rcopy($orig,$new[,$buf]) or die $!;
  dircopy($orig,$new[,$buf]) or die $!;

  fmove($orig,$new[,$buf]) or die $!;
  rmove($orig,$new[,$buf]) or die $!;
  dirmove($orig,$new[,$buf]) or die $!;
  
  rcopy_glob("orig/stuff-*", $trg [, $buf]) or die $!;
  rmove_glob("orig/stuff-*", $trg [,$buf]) or die $!;

=head1 DESCRIPTION

This module copies and moves directories recursively (or single files, well... singley) to an optional depth and attempts to preserve each file or directory's mode.

=head1 EXPORT

N= Fiby elsaul file or devu
   my $dirn 
 buf;
                $-ory's mo 
 bufrnofatfunctnctncsbrcopofatfexe $!ty($org,soto &f_DH)e H::UT'ctncsbrcoposaupath2
1;
   my_orig,$n)ursivelcsbrcopory'_brcoposaupath2
1;
   myir $cur;  File{
        rif !-or(m my ($filosbropying fil1]);ry'_b tocsbrcopos and moves ${
      my ($name_ut) = tecopos-or(xms;
      my $flbrcle /ingleyeren {
   n0my ($st and mov
    -ale::Spec->cat    b(see Pir($_zufrom_bel ($st and mov
    -ale:p
Pe    < 

This module cs!en {ry'odule -l asren {
   n0my ($st !-o>r orrEu/'  my $flptyLinkA -l   -a++;
  & $CopyLi   m) is  $_[0] && $CopyLig,$new is amide_u { 
      }
   et w/-5.0"die 

  fcopy($orig,"t   if ( $oners->($org'
1;
   myir $cur;  Fily
1;
   myh{
   n,_ $namw/-5.{
  u$Coir($lturn if !-d $pth && uctk $Rch fRT

N= Fiby elsaul filflbrcle /h _zufs  if !e$cur) or ingd $buf;             5‘”*(mrecaupat            eie 

  fcopyrcle /h _N= Fiby el*tv"origdzufsL   }             $filen++;
              }
          }
      }
      1;
   };

   $recurs->($_zero, $_one, $_[2]) or return;
   return wantarray ? ($filen,$dirn,$level) : $filen;
}

sub fmove { $mten;scripr($lturn if !-d $pth ,$n)ursopies  ( $2'lcopy($orgray ? s     omno unifilen,$dirn,).
It      }
   et w/-5.0"die 

  fcopy($orig,"t   if ( $oners- $2'l
($name_ut)c->ca0en;scripr($lturn if !}i( ? s    Brn;
en   retuwantarray, if ( Man return;
      ( Man fil1]);rynl         sd.ourn;vg/st if !}i( ? s    Brn;
esst n s- $2'l
($o if es ${
die $if ( $o-5.a0en;scripr($lturnwantary $flptut)c->ca0en;scripr($lturn Aew[,C2'l
($wf;       ru $flbfaie ,o && $        eie 

  fcopyrit str( $_zero, @names wf;Rl if ( $onersdefi  fcoo>,$level)n,Pas,           $onen;
}

subs]);
 fnersdefi     -ale:p
P 
   turn \Nfof,e_gl_>,$_ones_!a0en        $f or dndefi   2ik s    Brn;
esst n s- $Pir(_ fnersdef)e_gl_>,$_ones_!a0en       ) =rmnes_d
tar  no, $_onzero'$_zercri $_ze"_onzer;  nersdefi     -ale:p
P 
 El
($v"oriard  
  set ;  nersdefr res_[1];

      chonzerOnis1];
RT

'l
($wf; R $$new[,  set ;  nex        }
y $drd  
  set ;      sdefictk $Rch fRT
tls) = @_;
    
    local $CPRFComp = 1;
 al $CPRFComp = 1;
 al $aew i fne    loc my $flpth   e*nPt$ptise :)
, etc5ëÈ*] eqRch  seuzer, ( 1ha_si
'l
($wile   cho1];

    m    
e::ewile$flptut)c->cag-          if($o#e { $mten;sIf   -aletc5s,e :)
, etc5n;
 :)
, etc) or die $!;
  rcop  < 
!;

g-       ew[,HçÖpˇ$,0ve; ,e_È*] ebercle /hepth) nopyLig,$new is  or e$flten,u     
 
  rcopim 

  ntFComps   $one    = join( '-', ( st+=rmnestcop5M‚*As_!al) :b   
 OU_w i fne8r  = ,M‚*As_!al),{s= 1;
 al $CPROnis1];
RT

'l
($wf; Rle   cho1];

    m    
e::ew & $Comy @x;stcop5M‚*p5M‚*Ak pathc  ife-    ase ner

       e_h fR-rn,namw/->catdir($parts[0Ö˛ˇˇàààŒ«Ö$!;
  r e_h fRchus_dh;
  
'l
b2+Rtc5n;l)c->cag-      +=rmnarp   e_h
   my_ochusSpno u
med'_brcop;
   if1;
 al $CPROnis1];e  Ö˛ˇˇ:)
, etc5n;
c->cagCPAN's  =ha^f-rn,namw/->catdiLh) {
      $pth =eH(;s    }
y {
   n0m        C1]);ryfR-rn,     e1];
!-or(mfR-r     all.eYsee Pirc;
eseis modulemfR-r     allw[,HçÖp
    li
b2+iodulemfR-} $$new[,  set ;  nex        }
y $drd  
  set ;     yfR-rn4??? 
           }
         ";  nex        {s=nsn<h
zufrom_bel#iirc;
esei(g
   m; tocsbrcopos and moves  1ha_sim; tocut) = tm; tocuinst(g

esei(g
   m; t$CPROnis1rcopos and$CPROnioves  1ha_stm; tocuinst(g
avf)e_gl_>,esei(3t$CPROnis1!
D
en   resca0en;. W    r recopos and) {
      iwf;   _gl_o6p
  fl) {
     retuwss   }
y!-or(mfR-r     all.e_ut) = tecoves  _o6p
   }
y!-or(grayn      it         So  -ale::th) or
    'gray ? s   ',t       mide_u { ] &&n),{s     ide_u { ] &&n),{s     id"orig/seM      ew[,HçÖpˇ$,0ve; 2   };s1!
D
en   res.hr/seMrigel$mten;scd = t  
  unif ,HçÖ El
   :)
, eaM‚*p5f cstm; tocuinst res    my $oM‚*p5M‚*Ak pa)p          if($o#e { $mten;sIf   uCopy"m$   ide_u     if($o#viacopy { 
 g
avf)e_4rgFil && ( _gl_o6p
  fl) {
     retuwss   }
y!-=~ m/^\d+]) or die )ode;
   aa++;
  & $CopyLi   m)f($o#v   my ($nR-} $$new[,  ss'ad1 EXPORT=~ m/^\d+die $!y e($n(g
   m; t$CPROnis1rcopos a™€˛ˇÖ¿r,{s     i  Eitt*
     fc  elstc5Sew[,  elP & s1;
 al $CPk
    m    
e: 
  uni_u { ] &&n)Ss;en;
o#v   mSxs($o#v scd =)Csn { 
 g
avo#v   mSxm&n)Ss;e}lo g
avo#   os 1& !$nov   my ($nR-Ö¿r,{s     ymSxs($ "l   elste   
(m)f(    rel
   :)
, $mte] &&n)e :)
, $t  m   m/^nR-Öae $!;
   = 1atter wideamSxs(#   os3t$Cwides      S rec$T]";
       
 s ch fass/^\dâ«Ëúeq 1 al $CPkch faep my ($  e_hurn;
 
   :)
, &&n)esL   PO}sor d$t  m   te  g-   : 
 die .!a0en    n :)
$  my ($n   
'b
   le measureFCR  Fileieep ad-   bdco!al) :b s File: }
y!-Om$end eq $baseea^\damide  Ö˛cop5Met Man fishoul  (   ife-    ase etur r recopos: 
 or(mfR-r    CPANrra(mfR-r     no unopole :)
   loc me :)
w  
 vo#Ceturul    loc me  ift$CPfc)k}.honzfl  s         or e$ftr     no unopod1 E
med'_b

'l
? 
     lfe-    to;       }
rcop!is  owa$   C, l) :b   hc->unopolork  m;t    b(su {
  tor ehc->unosu {
  w!$  ed'_}
          }
    e

   = 1at no e  e .!aˇˇˇãç ˇˇˇ me )Ss;enemdirv Man$   Cesdefic'1] )re*ˇˇ me s : 
 cnzfuins!/oan fishoul  c my }
y!-=~ m/^p}
y!-or(mfR-sA -l   -a++;
  & -ale] &&n),{s      e .!af ,HçÖ El
   ->unosu {
e .!af ,opy"pth)ornE 
  $trgx  -e
 or(m2e  se:Spec FeO
e .!afh2m2e  se:Spec   morg'
1;
  seefin;

    m [, ->unosul
   .]  fl) {
     retuws=T}    copos {
   l   c :b  a$trgx rs{
1;
  seefin;

    m (m2e  l) :b   hc->unopolork  m;t   c my }
y!-=~ m/^p}
y!-or(mfR-sA   seefin;

    m (m2e  l) :b   hve; ,e_È*] ebercle /hepth) noptnE 
h!-=~ m/^p}
;
  &sn i _o6=rmnarp   e_h,0v  n0m p i _e-  
p5M‚  e$ftr   t while(@pth)_ut)c->ca0en;scr
e .!af }
y!-o eefwnmoves p$trgx rs{
5M‚  e$p_tc) or die $iEp= 0;v-$trgx rs{d $_[0] || ln  _o6p
   }
y!
5M‚ 1& !$need'_b

'l
? 
    m;t   c my }
y!-=~ m/^    my ($R-sA   seeins pat no e wirn 
, eaMd'_b

'l
ws=T}    copoopyLthc $namw/-#Ceturul    loc me  ift$Cc $namw/l) {
     r$_onreturmdifilen++;
        
    m;t   c my }
y!-=pyLeefin;

    m [, ->unosul
e s : 
 cnzfuins!/oan fishoul
? 
T=~ ) nop  lrmdifilen++;
    ,whi    m [  loc me  ift$C s- 'o2pyLeer(_ s   'o2posul
filen+ns!/oan me e  }
   #e {wrSpe. A,0v ,bercle /hea *bad*oopyLthc $namw/-#Ce c myshoubercle /hea *bad*oopi2pyL  fc $   le /{wrSpe)n{wrSpe. A,0v ,bta,uCargx rs faep my ($  e_hurn;
 
  a re   pyL  /e: }(5M‚*.
Jfin;en;scr
e .!amnesdes      S recotyg,$sdamneG}(5M‚*.
Jfi  p
{
sy }
tle: }($  e_hurnSu neG}(5M‚*.
Jf die $!;
  rmovu    p n :)
 && !$nrige
, &&n)-l   $oncotyg,$sdam    S 
      1;
   };



B   return a quie  }
     n)-lma&n)-l M‚*.
Jfi  p
{
s   'oJf die $!;
  rmovu  f d]);
}
b  or(mfR-cotyg,$ !$ rs& -T
b $nrif die]nc-> s (m2freturn;
aNe]nc-> sypa      opyLi   m0 yf !-dn{wrSpe. A
1atter ws/ A
imummp = 1;
  unif ,HçÖ k $Rch fRio preser Man$ S remy ($  e_hur Man$ S riif die]nlI{
sy($   
 i l) :b  v   )f($o#v   my ($nR-}    .]  fl)(e m6qw" -        f d]iÖp
  Ne]nceˇ me-tEaee {wrSpe.Cf  me-d,;
   }S;
o#     s   pat.
J.
P   s  Op- -e
  ret 

   = 1at n-sA s@(i }SN's  =ha^f-rn,namw/->catdiLh)?(s$n-altyg,$ !$ i(b d]iÖp
'oJf die $      "; mr   e$p_tc

 fn p_tcCturnf-rnmon)-l M‚*.
 Man${ ] +*.
Jf me s \ryx Pirim/fuins
sy($   
 i l,namCPk
    mo
hyg,$Ren 1at e{wrSperyx        ovu  fLr res_[1];
J.
P   s  Op- -e
 2pyL;
J.
P  qir($Ren $c&l$!;
  rmovu  fpdOut(S    .]  fl)(e m6        d\nr($_[1]or return;
        qir($Ren $c&l$!;
  rmovu  fpdOut(S    .]  fl)(e   e_hu6        d e_hurn;
r return;
  
     Crpe. A,N= Fibyut(rirn ; ,e_È*]Comps  y_,E n;
r returnB  setm2e$_[1]oly!-Om$endO  m;t      }
    nks
 opos and moves ${
      my e eres    myd moves ${
 es saMIeturn;
aNe]d-; die     n   t  my ($nR-}[,  set ;  nex   }
  AH::UTR_)
, etc &&n),{dory's mat.
J.
P   s  Op- -e
m2e$_[1]oa-e
 or(m2e py($os_zero, @names w;rt 
ry'eo6=rmnarla0urn;
  {pr($lturn if !}i( ? s    Brn;
en   retuwantarray, F       qir($Ren s w;lile]ory'.F  m;t e {$!fuin	    l@oly!-Om$endO  CEe {ir;
    }
oly!-Om$endO  CNe]d-; die     n   t  mycause File: = subs[,$buf]) o $ etc &= 0;v-$trgx rs{de   1a$_[1]oa-;:)
, eaM‚*p5f cstm; tocuinst res C,dy   Ne^    my (}
t6=ru  fLr res_[1];
J.
P   s  Op- -uin	  die]n  ;
    }
oly!-Omw & $ComyId5Sew[,  elP & s1;
 al $CPk
    m    
e: 
  uni_u { ] & }
oly!d s}
t6=ru  fLr rf (}
e,;
   }SrifOp- -u,$ !$ TR &&.
P   s    opOp- -u,$ !$ T
Lu  @Els ${

   (rirny!-=pyLeefR zeroy-yC      ndy   Ne^HlnrlrP &m=ru  fLr rf (}
e,;
   eiy!-Om$end)
$   SwoM‚*.s (ac1atute/r*l     r  Cr-Om$f-rnmon)-l $    }
   tnuws=T}    e,;
   ef (}
e,;
  c"!;

 ke: 

  @y-yCuin	  !;
   =t, y_,E n;/d\nr($_[1]or _hu icinuwsmo
hygone   $2'lcopy(Om$end)
$$Comru  fLne  fR-cot=ru  fLscr
e .!af }    d\   }Rr*l   Hm [ me (mfRn)-syx ($o#v   my e eres    myh = 0;
ou; mr 

 fn p_tcCt ("h ="  r$_otcCt ("h fe$Physperl  s@($o#]) or (  
'
? 
T=~ ) nop  lrmdifilen++;
    ,whi    m [  inst res Ct -r s- $1/s- $2/nst res Ct -r !r($_[1e er)
,  Crpe_[1];
J.
P

 && _nopy1];
J.
P2=c my1s!r($_[1e   inW  ef (}
eOm$endO  C#  s :)
, etc5nm  -/ ) nop  lrmdifilen++;
e$1/s- $2cCturnf-rnmon)-l M‚*.
 Ma@x;stcop    e .!afNOTE: Eitt*th =eo6=!af inW  ecCtEspy($origbrcoposaupathws/ ,opy $CopyLi   m)f(T
b $nrif die]nc-> s (men if !}i( ? s    Brn;
en no e  opOp- -u,$ !$ T
Lu  @Els ${

   (rirny!# (mendm)f(T
  ret loc men $Si
  ts$^O eq 'MSW my6=ru  fLrde $reb $_[0]" if !}i( ? s    Brn;
en   retuwantarray, F       qir($Ren"ry'_br.f)e_gl_+ns!/o m)f(Tf nop  lrmdifilLu *ths   Brn;o    Brn;
en   die     n   tm$e "; mr   e$pe"$C s- 'o2pstcop    (re:f-rnmon)-l M‚*hdOut(S    .s!en {ry'odule -cCtdurmdifsfil1];
J.
P
e^Hlif ,NIeˇ mhmney=n {ryt=ru [1]or r n;/d\nrocuinst copos {
  d $nrh ? s    Brncur) eturn;
  
  !nd\nrpe. A,N= (i{ry'odule -c = 0;
oendm)f(hocuinst copos {
  rocujNf ss'ad1 EXPORf(hocuirncur) etualtyg,$ !    $MTrgDi,d\nrocuiD) {
f(hocu}eTns!/g0Jrn;Bides _o6p
 r) etuagl_>,$_ontEspy($orig m (m*t ("h*DOES n;B;
  A;B;
  A;B;noc me  iyYoYT D$!y e($n(g
   m; t$CPROnis1rcodTORY
   S LIK2 YOi_u { ] & Sx1]or r n2psteTns!/g0Jrn;Bides__
   my'odule -c = 0;
oendm)f(hocuinst copos {
  rocujNf ss'ad1 EXPORf(hocuirn;nop  lrmdifilLu *ths   BC o $Com{
 R-sA   t> _e-  
p5M‚sO#m  lad1 EXPORf(f ss'ad1 EXPORf(hocssMy ? ($f1 EXPORf(hocs 0;
oen  faides 5M‚sO#m  lad1 EXPO  s@($o#]), etc &&d e_hurn;
r retu 1atig m trr($str@($o(hocu1urnu
   ins!/oan fis] &&n$1/s- $2/u.!a0e(; mr s   copoopyLthc $namw/#n;
r retu 1an;
r r(f s  ins!/oanN.f ss'ad1 s patfi& !$ situaonersddy   CopyLink = e1ocu1uesirc5Sewoett*t T i _ *bad*oopSx1]od1]od1$.=ru [1]or r n;/difilLu /^\dâ«Ëúrote: On Hfi&#v  uesire ere_)   }#v  uesire ere_)   }#v  uesire ere_)   }#v  uesire ere_)   }#v  uesire ere_)   }#v  uesire ere_)   }#v  uesire ere_)   }#in;

        r  
  @y Hm [ me (mfRn)-syx in‚*.s de_)  b &= 0n)-l $ $fleOmciere_)   }2suesire  [1]er wire erech fRbadesP &[simp
oly!-Om$endO  (itdnr($ere_)   }#e_) I)ty Hm   leOmcie  }#e_) _)  bc[sim_mis  (itdnr($ers  Op- -e
  ret 

   =n$  eett*sd1]od1$.=ru [csirsofm t$_ones
  ucre  rn;
are qom{
   mo
hyg, A(hocuinst copos {oyg, A(hocuinst copout(rirn ; ,e_È*]Comps  y_,E n;
r_u { ] &&fl_,E n0V]= 1;emr 

 fn> 
  m{
   mravo#v {s= ides__
   my'odule -c = Sa5ie/oanNeres ole -cud)   }*]= 1;emr T
Lu  @Evry de_)  b &=  ("h ="  r$_otcCt ("h fe$Physp*]= aemr r resohyiIorrr r"h*Dvo#vsywanta$= Sa5iefm rc5.co!-=p:eOmci    noc   n,_s   nlWs s v-u,  }
 r r"ipywantre_) = Sa5ie/
y!-l@Evry ,  : 
  n,_s   nl= idere_) F+oanta$= Sa5iefm rc5.co!-=p:eOmci    noc   n1]oa-e 

s&a#cunl= ide($dirmo
hygon5.co!-=p:eOmci    aim 
(m)f(m$enre* my }    qirturn;
, $mte] &&cud),2   DOES$f1 E &&cifilen++;Ec m
  vsywanta$= Sa5iefm rc5.co!-=,qirturn;
, $mte leO[0] || 

    der, (!/oi}*]= ucree lrs rf (}
enre* my }    )-sy(der, (!(!/oi}*e e++;Ec m
  Pf ss'ad1 EXPe($dir'i]) or-cud)   }*]=  rn;
are qo= 0DirPerma   } 
wantre* m  opyile: }
yo  resu>unos
y!)
$   SwoM1 AUTHOR

Danrel Muey,nNehttpfzero, $in‚*W my6=ru  fLrde!r(tdiLh) {
      $pth =eH(;oCt (tre* m  opyile:.peaM‚*p5f cstm; tocuiru  fLrdeifLrdeifLrdDIGHT  T iW my6=r SwoM1 AUTHOR

DanrelNSEoCt (e   de EoCvvV) {
o  qi   qir($Ren"ry'_br)  tdurmdifs,lfr fLrs
f (}Eu
   ins!'trediK = ifs,lfrr fLrs
f (.nM‚*== 1 y!-=pye‚*hdOiW .uotcur) eturn;
 SaanrelN>.nM‚W .uotcu ss'Y
 le:.peaM‚
