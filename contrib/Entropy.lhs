% Entropy Scanner
% Michael Stone <mistone@akamai.com>
% February 1, 2011

<!-- Copyright (c) 2011 Akamai Technologies, Inc. -->

We're going to prepare to process some tarballs of file samples, since we have
some nice infrastructure for creating them.

> import qualified Codec.Archive.Tar as T
> import qualified Codec.Archive.Tar.Entry as TE

The Tar library uses lazy bytestrings. However, the useful search primitives
for bytestrings seem to apply to strict bytestrings.

> import qualified Data.ByteString.Lazy as L
> import qualified Data.ByteString.Char8 as S

We also need a bunch of other utilities:

> import System (getArgs)
> import System.IO
> import Data.List
> import Data.Int
> import Data.Maybe
> import Control.Concurrent
> import Control.Monad

and we want to draw some charts:

> import Graphics.Rendering.Chart.Simple
> import Graphics.Rendering.Chart
> -- import Graphics.Rendering.Chart.Gtk
> import Data.Accessor
> import Data.Colour
> import Data.Colour.Names
> import Data.IORef
> import Control.Arrow

with GTK and cairo:

> import qualified Graphics.UI.Gtk as G
> import qualified Graphics.UI.Gtk.Gdk.Events as G
> import qualified Graphics.Rendering.Cairo as C
> import Graphics.Rendering.Chart.Renderable
> import Graphics.Rendering.Chart.Types

Next, we have some boilerplate for managing a GTK window, a drawing area, and a
keyboard input-processing loop:

> windowSize = 20000 :: Int
> windowSizeD = 20000 :: Double
>
> -- do action m for any keypress (except meta keys)
> anyKey :: (Monad m) => (String -> m b) -> G.Event -> m Bool
> anyKey m (G.Key {G.eventKeyName=key})
>     | any (`isPrefixOf` key) ignores = return True
>     | otherwise                      = m key >> return True
>   where ignores = ["Shift","Control","Alt",
>                    "Super","Meta","Hyper"]
>
> startGui :: T.Entries -> IO ()
> startGui es = do
>     G.unsafeInitGUIForThreadedRTS
>     -- G.initGUI
>     chartref <- newIORef Nothing
>     entriesref <- newIORef es
>     window <- G.windowNew
>     canvas <- G.drawingAreaNew
>     -- G.widgetSetSizeRequest window windowWidth windowHeight
>     G.onKeyPress window $ anyKey (showNextFrame window entriesref chartref canvas)
>     G.onDestroy window G.mainQuit
>     G.onExpose canvas $ const (updateCanvas chartref canvas)
>     G.set window [G.containerChild G.:= canvas]
>     G.widgetShowAll window
>     G.mainGUI
>
> showNextFrame window entriesref chartref canvas key = do
>     es <- readIORef entriesref
>     case es of
>         T.Done -> G.widgetDestroy window
>         T.Fail msg -> putStrLn msg >> G.widgetDestroy window
>         T.Next e es -> do
>             putStrLn $ T.entryPath e
>             writeIORef entriesref es
>             case key of
>               "s" -> return ()
>               _ -> do writeIORef chartref (viewEntryContent $ T.entryContent e)
>                       updateCanvas chartref canvas >> return ()
>     return True
>
> drawChart win renderable size = do
>     G.renderWithDrawable win $ runCRender (render renderable size) bitmapEnv
>
> updateCanvas chartref canvas = do
>     win <- G.widgetGetDrawWindow canvas
>     (width, height) <- G.widgetGetSize canvas
>     let sz = (fromIntegral width,fromIntegral height)
>     mchart <- readIORef chartref
>     when (isJust mchart) (drawChart win (fromJust mchart) sz >> return ())
>     return True

The magic happens in the function from tarball entries to charts:

> viewEntryContent (TE.NormalFile body size) = let
>     indices = [0..(fromIntegral (size - 1))]
>     doubleIndices = [0..windowSizeD]
>     --values = map (fromIntegral . fromEnum . (S.index . S.concat . L.toChunks $ body)) indices
>     width = 512
>     values = map (entropy . L.take width) (take ((fromIntegral $ L.length body) - (fromIntegral width)) (L.tails body))
>     extendedValues = (values ++ (replicate (windowSize - (length values) - 1) (0 :: Double)) ++ [8.0])
>     dotplot = plot_points_style  ^= filledCircles 1 (opaque blue)
>             $ plot_points_values ^= zip doubleIndices extendedValues
>             $ plot_points_title  ^= "am points"
>             $ defaultPlotPoints
>     layout = layout1_title ^= "Amplitude Modulation"
>            $ layout1_plots ^= [Left (toPlot dotplot)]
>            $ defaultLayout1
>   in case (size > 256) of True -> Just (toRenderable layout); False -> Nothing
> viewEntryContent _ = Nothing
>
> entropy w = ent where
>     s      = S.concat . L.toChunks $ w
>     len    = S.length s
>     groups = S.group $ S.sort $ s
>     keys   = map (S.take 1) groups
>     counts = map (S.length) groups
>     probs  = [(fromIntegral c) / (fromIntegral len) | c <- counts]
>     ent    = negate $ sum $ [p * (logBase 2 p) | p <- probs]

Finally, we wire up all the pieces:

> main :: IO Int
> main = do
>   args <- getArgs
>   txt <- L.readFile (fst args)
>   startGui $ T.read txt
>   return 0
