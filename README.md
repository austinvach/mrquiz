# Electronic Learning Game
Mr. Quiz is a DIY device similar to the original [GeoSafari](https://en.wikipedia.org/wiki/GeoSafari). It supports legacy GeoSafari content as well as brand new content that will soon be available for purchase.

## Project To-Dos
- [x] Create POC that reads from [Codes.h](Codes.h) and accepts input from the user.
- [ ] Randomize ordering of Q/A pairs at the start of a new game.
- [ ] Add option to toggle between GeoSafari mode and standard mode (GeoSafari mode question numbers are not sequential but map to the lights on the original unit).
- [ ] Add scoring (3 tries per question with fewer points awarded for each mistake).
- [ ] Allow user to specify the time to respond at the start of each game (adjust sleep timer accordingly).
- [ ] Add two player mode.
- [ ] Update [Codes.h](Codes.h) to include all known GeoSafari codes and the associated Q&A pairs.
- [ ] Enable deep sleep when disconnected from power during "pretend sleep".

## Shopping List
1. [T-Display Keyboard Kit](https://a.aliexpress.com/_mNxfdco)
   - If you don't have access to a 3D printer, [purchase the version that has a shell](https://www.aliexpress.us/item/3256803403391540.html).
1. [3.7V Battery with Micro JST 1.25mm Connector](https://a.co/d/82xNgZM)
   - Any 3.7V lithium ion battery with at least 1000 mAh and a 1.25mm JST connector should be fine.
   - If you want the battery to fit inside the keyboard kit, maximum dimensions are 40 x 52 x 11mm. 
1. [3D Printed Case](3D/keyboard-case.stl)
   - The [3D](3D) folder contains individual STLs for the upper and lower sections in case you want to print them in separate colors. 
3. [3D Printed Battery Support](3D/battery-support.stl)

## Device Pics

![IMG_7676](https://user-images.githubusercontent.com/7928540/228622020-f4648935-bd6c-4215-b299-7fe03483bc62.jpg)

> Much of this project was based off the [LNPoS project](https://github.com/lnbits/lnpos/tree/main). Shoutout to [LNbits](https://github.com/lnbits) for the cool work they do!
