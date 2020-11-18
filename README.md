# Hearts 1.5.5 (Linux)
The card game Hearts for QT 5.x and liballegro 5 (for sounds support)

## 1. Requirements:
- Qt Creator (suggested)
- QT Widget 5
- Liballegro 5

## 2. Install: (under ubuntu 16.04)
<pre><code> sudo apt-get update
 sudo apt-get install git
 sudo apt-get install qtcreator 
 sudo apt-get install build-essential
 sudo apt-get install qt5-default
 sudo apt-get install liballegro5-dev</code></pre>
   
<p>After that you need to download Hearts repertory</p>
<pre><code> git clone https://github.com/Rescator7/Hearts.git</code></pre>
   
### Option #1: (without QtCreator)
<pre><code> cd Hearts
 qmake Hearts.pro
 make</code></pre>
   
### Option #2: 
   <p>Start up qtcreator</p>
   <pre><code>qtcreator</code></pre>
   
   - Open Project (Locate the directory Hearts, and open Hearts/Hearts.pro)
   - Settings file for "Hearts" from a diffrent environment. Choose: Yes.
   
   <p>Maximize window (on the top bar select)</p>
   
   - build -> run QMake
   - build -> build all (wait for about 5 mins to compilation process)
   - build -> run (if you want)
   
   <p>After that the executable of Hearts should be located in a directory like build-Heart-Desktop-Debug/Hearts</p>
   
## 3. Licenses: 
 - MIT for the software
 - Cards set: https://github.com/htdebeer/SVG-cards/blob/master/LICENSE
 - Sounds are CC 3.0 and 1.0
 - Icons: (read credits)
<p>(read credits for more informations)</p>

![screenshoot](https://github.com/Rescator7/Hearts/blob/master/screenshot/SCR1-hearts-1.5.5.jpg)
