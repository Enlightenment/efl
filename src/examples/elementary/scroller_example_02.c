//Compile with:
//gcc -g scroller_example_02.c -o scroller_example_02 `pkg-config --cflags --libs elementary`

#include <Elementary.h>

static const char *text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse bibendum, nibh eget vehicula tempus, libero dui aliquam dui, eget pellentesque nulla est id mi. Sed orci nisl, mattis vitae dignissim scelerisque, imperdiet vel arcu. Integer ut libero id massa commodo dignissim sed at orci. Quisque sollicitudin eleifend malesuada. Donec orci orci, mollis quis euismod vel, tincidunt nec ipsum. Sed ut nulla ligula, ut commodo tellus. Duis non felis in odio fringilla venenatis eget in metus. Mauris molestie ipsum tortor. Suspendisse potenti. Curabitur facilisis ultricies cursus. In euismod viverra risus sit amet pharetra. Sed mauris lorem, volutpat a vehicula quis, interdum nec erat. Suspendisse auctor bibendum fringilla. Curabitur tincidunt sem risus.\
                           <br>\
                           Vestibulum laoreet, leo ut sollicitudin viverra, odio enim tempor est, at ullamcorper augue massa sit amet lacus. Phasellus eget orci sit amet sem condimentum semper. Quisque imperdiet purus vitae lacus bibendum ultricies sollicitudin metus interdum. Phasellus aliquam sem vitae mi imperdiet eu cursus ipsum euismod. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Aliquam sed tellus non ligula commodo porta sagittis id augue. Vestibulum faucibus vehicula purus, sit amet porttitor ligula lacinia ac. Morbi ipsum libero, rhoncus id dapibus tincidunt, sagittis eget tellus. Proin vehicula accumsan velit cursus pharetra. Nullam pellentesque erat fringilla diam elementum fringilla. Nam lobortis ultricies suscipit. Pellentesque ut ipsum a nisi placerat rutrum id vitae justo. Fusce blandit ligula mollis est auctor scelerisque. Suspendisse luctus nibh vitae nibh congue rutrum. Nunc vulputate diam vel est lacinia a imperdiet purus scelerisque. Maecenas et nisl mauris. Aenean molestie dolor nec lacus laoreet ac dictum sem condimentum. Integer sit amet enim lorem, vitae tincidunt justo. Ut vulputate ullamcorper tortor, eu pharetra nulla aliquet in. Vestibulum ultrices sapien a ligula venenatis eu gravida lacus fermentum.\
                           <br>\
                           Etiam dignissim libero sed velit rhoncus ultricies. Aenean porta erat sit amet orci auctor auctor. Suspendisse aliquet arcu ut est ultrices ut venenatis dui scelerisque. Nulla vitae leo massa. Nullam scelerisque felis lectus, sed faucibus mi. Pellentesque hendrerit libero nec metus viverra lobortis. Nulla facilisi. Sed fermentum aliquet velit non imperdiet. Vestibulum nec ante imperdiet dolor convallis ornare sit amet interdum mi. In tellus lorem, lobortis vitae consectetur ac, mattis sed enim. Vivamus nibh enim, convallis sed rutrum sit amet, scelerisque ullamcorper ipsum. Mauris dignissim posuere sagittis. Fusce volutpat hendrerit augue ut dapibus. Aenean eu hendrerit lorem. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae;\
                           <br>\
                           Nullam orci eros, mattis at gravida sed, iaculis et dolor. Pellentesque sagittis sollicitudin malesuada. Sed tincidunt, sapien in mollis semper, quam enim gravida neque, eu blandit lacus ligula sit amet nisl. Aenean quam nulla, dapibus at venenatis vel, accumsan id dolor. Nam varius urna id nisi tempus ut dignissim nunc ultricies. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Integer lacus tortor, pellentesque auctor venenatis sed, laoreet eget erat. Nunc erat est, pharetra vestibulum luctus et, commodo eu quam. In hac habitasse platea dictumst. In hac habitasse platea dictumst. Nam posuere diam vel felis scelerisque iaculis. Maecenas laoreet ipsum ac orci tempus lobortis.\
                           <br>\
                           Sed id massa metus, lobortis porta ipsum. Nullam pellentesque augue sit amet neque hendrerit vehicula eu quis elit. In nec ante massa. Praesent at pulvinar tortor. In laoreet molestie lectus, ut interdum lorem lobortis vitae. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Vestibulum ligula sem, interdum eget gravida in, fringilla in sem. Phasellus orci felis, semper rhoncus pharetra a, pellentesque egestas elit. Ut bibendum, est vel gravida tincidunt, enim massa consectetur arcu, vitae pretium sem erat ac nisi. Ut eget erat nisl, eget laoreet arcu. Vivamus risus ipsum, aliquam non ornare et, ornare vel libero. Cras vel ipsum ut magna sagittis interdum at id risus. Morbi justo nulla, volutpat at molestie vel, mollis vitae nisl. Curabitur non erat elit, eu vehicula turpis. Sed eget feugiat neque.\
                           <br>\
                           Nulla facilisi. Donec sit amet ante sed lacus adipiscing hendrerit. Vivamus enim nibh, mollis eget elementum vitae, congue non sapien. Maecenas interdum magna vel velit faucibus tempus. Nullam nec nunc risus, lobortis imperdiet magna. Suspendisse potenti. Duis fringilla sodales massa eget egestas. Ut tincidunt adipiscing ante, quis consequat mauris venenatis vitae. Suspendisse mattis sollicitudin accumsan. Duis varius ornare dui ac interdum. Sed molestie elit sit amet dolor varius vel congue nibh varius. Donec semper risus placerat dolor blandit tempus. Etiam id leo sit amet nulla gravida suscipit ut ut metus. Curabitur non elit sit amet sem dapibus interdum ut dictum nunc. Integer ultrices tincidunt faucibus. Nam mollis turpis vitae nulla pulvinar in sodales purus lobortis. Cras nisl lectus, tincidunt a suscipit id, sodales nec nulla. Nulla faucibus pretium feugiat. Nam ullamcorper, ante nec ullamcorper eleifend, nisl est dictum magna, ac rhoncus quam lacus eu neque.\
                           <br>\
                           Cras sodales eros eget ligula porttitor tincidunt. Vivamus ac justo non nulla placerat tempor. Quisque ullamcorper venenatis lacus id mollis. Fusce tempor dui ut justo pretium at volutpat velit malesuada. In tellus diam, mattis sit amet viverra eu, porta sit amet metus. Pellentesque ante risus, dictum a rhoncus ut, hendrerit sed nisi. Donec congue dolor eu orci imperdiet id vulputate nulla eleifend. In vel risus urna, sit amet laoreet augue. Donec fringilla massa nec augue scelerisque eget fermentum augue mollis. Maecenas eu eros nulla. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam lobortis, dui id sagittis vestibulum, magna ipsum venenatis sem, vel dapibus leo enim a elit.\
                           <br>\
                           Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Proin lectus felis, aliquet id euismod ac, vestibulum vel leo. Donec venenatis, nunc ornare dictum aliquet, dui purus ullamcorper massa, nec vehicula odio orci nec tortor. Cras sapien ante, laoreet ut placerat nec, mattis in velit. In hac habitasse platea dictumst. Vivamus viverra consectetur augue lobortis dignissim. Ut est nibh, varius at fringilla vitae, viverra at massa. Vivamus mattis bibendum lacus, at dignissim sapien auctor ac. Donec quis mauris lacus. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Phasellus vel turpis quis ipsum mattis congue. Integer gravida semper eros vitae tempus. Maecenas volutpat mi vitae purus feugiat volutpat. Praesent ut diam lorem. Maecenas ultrices, felis a vestibulum auctor, neque dui tristique enim, id mollis nunc arcu eget dui. Sed rhoncus, ligula non elementum aliquet, ligula nisi lacinia sapien, sed mattis ipsum nibh et felis.\
                           <br>\
                           Nam libero diam, aliquam vel pharetra eu, fringilla sit amet lacus. Nullam varius eros vitae arcu porta ut accumsan massa ultrices. Donec at convallis magna. Proin sit amet dolor et enim fermentum blandit. In urna ligula, molestie lobortis varius eget, aliquam id justo. Maecenas ac viverra lacus. Quisque eros nisl, varius a sollicitudin at, auctor sit amet ligula. Nam in erat non leo volutpat pulvinar. Nulla quis turpis sit amet augue pharetra placerat non vel nunc. In at justo felis, ultrices congue lacus. Etiam sed est velit. Sed elementum, justo sit amet placerat porttitor, elit urna consectetur neque, eget scelerisque dui nisi quis magna. Donec condimentum sollicitudin augue eu volutpat. Etiam vel tempus mauris. Donec et leo eu nisl tempus malesuada sit amet in massa.\
                           <br>\
                           In in nulla eu justo venenatis hendrerit a ac ante. Mauris sollicitudin metus at eros volutpat eu tincidunt justo pharetra. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Suspendisse potenti. Nunc purus velit, ornare sit amet pretium eu, congue tincidunt nunc. Maecenas sollicitudin augue sed magna tempor non rhoncus lorem pulvinar. Mauris scelerisque fringilla accumsan. Aliquam et ligula nec lectus dapibus hendrerit. Etiam cursus metus eget odio consectetur sit amet egestas turpis pulvinar. Aenean nec risus vel ante condimentum condimentum. Quisque convallis nunc porta mauris vulputate imperdiet. Etiam varius, nunc sit amet hendrerit convallis, est purus cursus libero, et varius eros leo eget est. Vestibulum a augue nulla, a convallis lacus. Mauris gravida metus ac enim mattis tempor. Etiam accumsan tempor mattis. Fusce dignissim urna at leo pellentesque a blandit nisl faucibus. Fusce consequat auctor nisi ut sodales. Etiam imperdiet velit quis magna consequat placerat. Sed nisl purus, condimentum ac adipiscing vitae, consectetur placerat neque. Morbi molestie sapien sed ante sagittis ullamcorper vel sed erat.\
                           <br>\
                           Nunc at risus nec dui viverra imperdiet vitae quis velit. Mauris ornare consectetur lorem, vel blandit risus lacinia vitae. Integer gravida, neque porta malesuada malesuada, leo urna egestas urna, at elementum ligula ipsum non diam. Maecenas lobortis, est eget interdum eleifend, tellus leo suscipit orci, sed fringilla eros nisl at erat. Proin bibendum dictum varius. Sed lectus leo, feugiat quis sollicitudin sit amet, egestas quis lacus. Donec vel est nec leo porttitor sagittis non quis lectus. Nunc in ullamcorper urna. Mauris sit amet porttitor dolor. Praesent eu dui nisi, quis aliquet tortor.\
                           <br>\
                           Duis auctor ornare nibh, non mollis augue cursus vitae. Suspendisse accumsan commodo felis, vitae convallis eros volutpat a. Fusce tincidunt, purus laoreet bibendum dignissim, justo felis iaculis velit, ac feugiat tortor turpis non velit. Integer tristique ultricies dolor, at condimentum nunc rutrum vel. Nam a velit nisl, quis hendrerit erat. Nulla sit amet metus pellentesque nulla consectetur venenatis. Nunc auctor, dolor eu ultricies adipiscing, augue dui sollicitudin arcu, ut dictum nisi felis ac leo. Donec tempor erat at nisi aliquam pulvinar. Etiam rutrum massa vitae libero gravida ac ornare justo molestie. Pellentesque non nisl varius nisl laoreet convallis sed porta sem. Donec rhoncus sapien tempus ligula placerat interdum.\
                           <br>\
                           Donec varius posuere lorem, a fermentum est molestie eu. Maecenas metus ligula, faucibus ac pharetra vel, accumsan quis mauris. Nulla ultrices, nunc ut tincidunt suscipit, elit dolor bibendum ante, at gravida nisl mi dictum purus. Integer vulputate facilisis nisi, quis porttitor mi iaculis sed. Aenean semper facilisis quam, ut dictum mauris vehicula sed. Sed non sem quis magna ornare egestas ut quis velit. Suspendisse ut quam est, euismod facilisis magna. Etiam mattis pulvinar augue, eget tristique purus porttitor non. Sed egestas dui ac odio auctor convallis. Sed posuere ornare iaculis. Aenean ac nulla ipsum, molestie facilisis eros. Phasellus tincidunt nunc diam. Aliquam malesuada, velit et tincidunt interdum, dolor mi hendrerit velit, quis facilisis ligula dui ac elit. In viverra pretium enim vel tempus.\
                           <br>\
                           Morbi at adipiscing nisi. Fusce vel turpis vel libero commodo mollis id iaculis urna. Nullam et elit non lectus egestas semper. Curabitur sodales turpis sit amet metus fringilla quis fermentum orci lacinia. Aenean lacinia feugiat hendrerit. Vivamus nec nisi nec neque lacinia rhoncus ut id erat. Suspendisse luctus tortor sed arcu interdum sit amet aliquet neque vehicula. Curabitur eget nunc vitae libero vulputate dictum. Etiam volutpat fermentum felis fringilla euismod. In scelerisque gravida dolor id molestie. Nam faucibus sapien id neque vehicula semper. Nulla egestas urna vitae sem scelerisque rhoncus. In hac habitasse platea dictumst. Nulla vitae lacus in diam aliquet sollicitudin vel nec leo. Sed ornare pellentesque nibh, a malesuada erat imperdiet et.\
                           <br>\
                           Maecenas adipiscing diam a urna interdum pharetra. Phasellus vitae turpis urna, ac consectetur sem. Sed porttitor nulla in nulla mattis tincidunt non eu lacus. Aliquam quis metus turpis. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Pellentesque at sapien quis purus bibendum lobortis. Donec in tincidunt nisl. Phasellus interdum tellus non leo aliquet ut vulputate nisl aliquam. Aliquam faucibus lorem ut lorem convallis non lobortis felis consequat. Phasellus nec leo ac turpis egestas laoreet. Cras venenatis odio et diam faucibus pellentesque. Quisque eu orci magna, quis lacinia tellus.\
                           <br>\
                           Aliquam nisl purus, aliquam quis tristique nec, varius eget risus. Nam sed ipsum leo. Proin congue sapien quis libero porta ornare. Mauris vel orci odio. Nam nec felis nibh, non congue odio. Duis vel mattis enim. Maecenas at dui eget ipsum pharetra consequat ac in massa. Proin fringilla arcu non enim feugiat accumsan. Proin tincidunt ligula mattis risus hendrerit in scelerisque risus aliquam. Vivamus libero sapien, ornare ut fringilla ullamcorper, sodales mattis purus. Quisque mauris sem, fermentum vitae adipiscing non, ultricies id mauris. Donec tincidunt, odio vel luctus hendrerit, dui mauris posuere erat, at interdum arcu nunc tincidunt dui. Pellentesque diam orci, malesuada a pellentesque nec, rhoncus at tellus.\
                           <br>\
                           Morbi in lacus nunc. Proin congue nisi at nunc gravida imperdiet. Duis et elementum arcu. Cras pretium, tortor in ultrices pharetra, sem ante condimentum elit, at dapibus augue lectus ac est. Pellentesque gravida rutrum pulvinar. Nulla nunc odio, dictum id tempus sit amet, faucibus volutpat nunc. Vestibulum ut ultricies tellus. Donec eget tortor metus, pellentesque placerat eros. Suspendisse ut lacus ipsum, sed fermentum est. Sed interdum ornare augue, sit amet bibendum ligula molestie eget. Aenean ac elit diam, eget ultricies nunc. Nullam quis nibh vitae diam pellentesque viverra. Aliquam ultricies elementum enim tristique malesuada. Vivamus rhoncus faucibus nunc, a pharetra enim sollicitudin ac. Suspendisse ultricies sodales metus vel rhoncus. Integer feugiat euismod molestie. Phasellus iaculis magna imperdiet erat consequat et pretium orci tincidunt.\
                           <br>\
                           Morbi tristique cursus consequat. Morbi posuere lacinia odio quis auctor. Nulla dui nisi, tincidunt condimentum aliquam sed, adipiscing cursus sem. Nullam eleifend lacus tempor nisl semper quis cursus velit vehicula. Morbi aliquam, sem at sollicitudin laoreet, nibh erat congue nisi, a pulvinar nulla orci vulputate orci. Morbi eu orci enim, quis aliquam ante. Nam ultricies laoreet varius. Proin vehicula sem vitae dolor ullamcorper et rhoncus lacus cursus. Nullam sit amet mauris sapien, vitae condimentum justo. Cras eu arcu magna. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Phasellus vulputate erat ac diam sagittis dignissim non nec tellus. Vivamus tristique quam vel velit mollis in congue nulla malesuada. Phasellus hendrerit egestas urna vitae egestas. Morbi vitae enim magna. Proin quam tellus, tincidunt ut mollis ut, semper a mi. Vivamus nec pharetra purus. Aenean at lorem mauris, sed facilisis quam. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas.\
                           <br>\
                           Cras tempor, enim in ullamcorper dignissim, elit eros sodales augue, vel molestie velit tellus sed risus. Suspendisse fermentum, augue id facilisis hendrerit, magna dolor euismod lectus, gravida consequat neque turpis lacinia nisi. Etiam id mi nulla, nec ultrices leo. Aliquam et mi tortor. Duis ullamcorper enim a tellus gravida ornare. Sed interdum, felis eu consequat sagittis, lectus neque ultricies velit, eget egestas ligula quam et mauris. Nunc eu quam enim. Duis tempus, arcu at tristique tristique, justo est gravida orci, vitae hendrerit risus tellus eu urna. Nunc vitae nunc vitae augue malesuada viverra. Etiam nec diam sit amet augue commodo pharetra. Fusce lectus urna, aliquam eget rhoncus tempor, ultrices et lacus.\
                           <br>\
                           Phasellus tincidunt, diam et ultricies ornare, ante tortor pretium elit, vitae viverra urna ipsum vel ipsum. Duis semper magna ac mauris rutrum facilisis. Suspendisse mi velit, auctor at rutrum sit amet, lacinia at orci. Phasellus diam magna, vulputate ac accumsan non, ultricies et lorem. Etiam nisi purus, tempor vel aliquet in, egestas sit amet massa. Quisque auctor, dui quis aliquet condimentum, ligula urna rhoncus mi, non vehicula velit sapien eu libero. Nam et elit elit, nec semper enim. Sed quis nulla ut ipsum consequat placerat. Nullam eros risus, congue et tincidunt et, ornare ac nibh. Aliquam vitae neque ac orci tincidunt pharetra. Quisque ac augue iaculis nisi lacinia laoreet. Etiam dolor magna, convallis eu bibendum vel, pellentesque non mi. Etiam eget nibh in metus venenatis hendrerit sit amet ac ligula. Nullam laoreet lobortis fringilla.\
                           <br>\
                           Donec non nunc sit amet sem consectetur sodales. Donec nisi ipsum, imperdiet sed commodo nec, placerat ac eros. Sed ac orci sit amet nisl molestie vestibulum. Nam sollicitudin mauris ac eros consequat iaculis. Nam aliquet lobortis ligula, at tempor libero hendrerit id. Vivamus at nisl eget sem dapibus dapibus quis at enim. Nam eget nisi urna. Nam ultrices, nibh ac vehicula dapibus, ante odio vestibulum ante, ut volutpat orci lectus in mi. Aliquam venenatis fermentum condimentum. Aliquam ac lacus elementum nisl molestie auctor ullamcorper sed orci. Nullam tristique metus sit amet purus commodo vulputate. Ut ante turpis, congue non lobortis non, vehicula non diam. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Mauris lobortis magna ac est facilisis in congue orci semper. Etiam et lacus eget elit viverra dictum eu ut neque. Vestibulum diam erat, posuere ut sagittis at, ultrices sed turpis. Cras justo risus, tempus eu facilisis vel, tempus scelerisque est. Vestibulum sagittis mauris ac turpis malesuada tincidunt. Proin et eros eget augue pellentesque tristique non in justo. Integer in leo sit amet lectus convallis rhoncus vitae eu quam.\
                           <br>\
                           Nulla eleifend hendrerit sem, vel feugiat mauris pellentesque vel. Vivamus ut lectus enim. Vestibulum nulla lacus, vestibulum et viverra sed, malesuada vitae urna. Integer consectetur, ante eget cursus tristique, lectus felis dignissim ipsum, non mollis arcu dui eu lacus. Integer sed ante ut magna euismod molestie sit amet ut leo. Quisque nec leo dui. Nunc sit amet ligula mi. Etiam sagittis scelerisque neque, ac lobortis lacus feugiat pellentesque. Donec ultricies est vel est tempus non volutpat turpis feugiat. Suspendisse potenti. Vestibulum et velit vitae nisi eleifend tincidunt quis eu odio. Nunc a massa arcu, ut ultrices sem. In eu tempor enim. Praesent aliquam arcu eleifend metus accumsan non pharetra dui ornare. Cras lorem leo, scelerisque sed suscipit at, semper porta magna. Vestibulum pellentesque, erat congue euismod fermentum, mauris purus blandit ligula, at luctus nisl magna in metus. Sed pulvinar, ipsum eget hendrerit egestas, lacus odio posuere tortor, in euismod libero purus id orci. In hac habitasse platea dictumst. Curabitur feugiat semper varius. Aliquam mauris metus, rhoncus in rhoncus non, blandit eget magna.\
                           <br>\
                           Cras sapien odio, facilisis id accumsan ut, laoreet sit amet felis. Duis et velit sed tellus volutpat porttitor. Praesent sagittis hendrerit sapien sit amet scelerisque. Vivamus vitae justo at mi venenatis cursus. Nulla est purus, lobortis vel rutrum vel, aliquam in ligula. Sed tortor mauris, pulvinar eget luctus id, faucibus at justo. Phasellus massa quam, tincidunt vel condimentum a, varius vitae tortor. Cras gravida ullamcorper mauris, vel condimentum neque sollicitudin sit amet. Nullam quis justo elit, facilisis dapibus orci. Ut ullamcorper, velit a fermentum elementum, erat erat posuere lacus, egestas iaculis nulla ante eget nibh. Nam ullamcorper leo at diam consectetur molestie. Suspendisse porttitor, eros ac euismod vulputate, tortor libero tempor urna, non pharetra nisl nunc quis sapien. Mauris commodo venenatis risus, eu commodo neque ultricies adipiscing. Nullam ac nunc tortor. Sed consectetur placerat luctus. Curabitur non risus lacus, tristique fermentum quam. Pellentesque viverra, nulla in imperdiet accumsan, eros erat ultrices enim, at adipiscing massa sapien vitae elit. Praesent libero tellus, pharetra et placerat sed, facilisis ut mauris. Phasellus nisl justo, consectetur quis eleifend eget, pulvinar eget erat.\
                           <br>\
                           Integer placerat tellus vitae leo luctus ac cursus velit varius. Fusce pulvinar iaculis purus, placerat blandit est imperdiet sit amet. Nulla tincidunt pellentesque tortor ac consequat. Morbi hendrerit, elit non sollicitudin lobortis, massa nunc rhoncus eros, vel egestas felis dolor non nunc. Praesent vel sagittis elit. Mauris pretium leo molestie purus ornare at volutpat risus suscipit. Integer quis orci et magna dapibus gravida. Maecenas metus est, egestas eu interdum nec, suscipit et turpis. Pellentesque eu sagittis ligula. Aenean interdum sem purus. Maecenas interdum nibh aliquam libero aliquam laoreet. Morbi mollis, ligula id vehicula malesuada, velit ipsum euismod nisi, quis mattis turpis neque eu odio. Praesent ultrices vestibulum nisl, non eleifend dui convallis vitae. Pellentesque id sem enim. Morbi eu turpis massa, non fringilla erat. Mauris at metus ut urna luctus rutrum vel in lorem. Donec vel facilisis nisi.\
                           <br>\
                           Phasellus sed facilisis mi. Suspendisse mollis sapien vitae lectus tincidunt a molestie nisl feugiat. Etiam sagittis lectus ut risus tempus consectetur. Phasellus non dolor massa, ac consequat mauris. In suscipit, purus sed pretium lacinia, odio tortor vestibulum leo, ac facilisis libero est eget neque. Nullam vel sem sem, vel egestas purus. Etiam tempus sem sit amet purus tempus hendrerit. Sed bibendum lacinia dapibus. In magna ipsum, molestie id mattis ut, posuere semper velit. Fusce ut lectus felis. Vivamus sagittis aliquam leo, ut facilisis tortor iaculis vel. Praesent egestas placerat est, ut congue justo vestibulum sed. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Ut id pellentesque nisi. Fusce bibendum urna eget leo porttitor tincidunt. Nunc blandit condimentum hendrerit. Nullam placerat, risus sed condimentum rhoncus, nisl neque imperdiet metus, condimentum lacinia ipsum elit non felis. Donec sit amet felis et diam adipiscing mattis.\
                           <br>\
                           Nulla et nisi ante. Duis ac nisi ut nibh mattis fringilla. Duis sapien lectus, consequat ut feugiat a, feugiat ut nisi. Aenean enim leo, gravida non bibendum id, vulputate eu magna. Suspendisse sed tortor faucibus eros sollicitudin fermentum. Ut cursus tellus ut dolor fermentum et iaculis mauris egestas. Donec sit amet massa leo, eget dignissim ante. Nam dignissim massa a risus lacinia at varius ligula tempus. Vivamus dignissim sem in enim consectetur sagittis. Aenean aliquam hendrerit urna eu aliquet. Donec luctus suscipit odio, ut accumsan neque suscipit vitae. Integer hendrerit facilisis orci feugiat commodo. Praesent vestibulum orci et turpis vehicula vulputate. Sed in purus sit amet tortor sagittis lacinia. Nam nibh justo, rhoncus et ultrices non, venenatis nec ligula. Suspendisse eget lacus sapien, et consequat erat. Ut auctor condimentum magna, ac condimentum magna laoreet volutpat. Sed urna urna, ultricies eget venenatis nec, adipiscing nec tellus.\
                           <br>\
                           Praesent viverra dui eget nisl lobortis in malesuada nunc sollicitudin. Aenean bibendum scelerisque metus ac facilisis. Donec est neque, egestas eu aliquet commodo, dictum eu metus. Quisque vel purus in sapien tristique euismod vitae nec eros. Aliquam tincidunt viverra odio. Aenean euismod lectus rutrum eros tempus ut lacinia eros pharetra. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Cras at sapien augue. Sed posuere, dolor nec ullamcorper venenatis, est est pulvinar urna, vel placerat nulla ligula sed eros. Pellentesque nec enim magna. Ut pharetra quam a lectus dapibus eu tristique ante porta. Aliquam vel augue eu risus mollis blandit sit amet vel arcu. Donec lobortis lacinia magna nec aliquet. Suspendisse fermentum magna eget est posuere nec pellentesque enim accumsan.\
                           <br>\
                           Vivamus commodo urna sit amet enim iaculis vel accumsan odio interdum. Etiam est lectus, fringilla in fermentum nec, imperdiet sit amet sapien. Suspendisse vitae lacus vel elit convallis iaculis non eu tellus. In nec dolor diam. Mauris sed tortor id ligula vestibulum mollis sit amet a neque. In hac habitasse platea dictumst. Donec a velit vel nunc ornare euismod sit amet vitae risus. Sed ultrices semper nibh, quis gravida magna dapibus ac. Fusce ut ligula velit. Aenean non mollis augue. Proin ut ligula nisi. Duis nec consequat dui. Praesent vel mauris orci. Nullam est nunc, ultrices ultricies suscipit et, adipiscing in augue. Sed porttitor mi in nunc euismod tincidunt.\
                           <br>\
                           Quisque laoreet congue augue quis cursus. Donec sed nisl odio. Etiam rhoncus fermentum lacus, quis pharetra urna semper vitae. Quisque non nisi at nunc volutpat porttitor. Donec ligula massa, suscipit non facilisis vitae, lacinia sit amet lacus. Fusce vel turpis orci. Etiam quis ligula eu nibh tincidunt posuere. Mauris sit amet quam leo, porta interdum diam. Nullam purus metus, facilisis quis ullamcorper vitae, mattis ultrices ante. Sed tincidunt lorem aliquet magna ultrices eleifend. In non nisi orci. In imperdiet, tellus eget fringilla elementum, lorem magna faucibus libero, id placerat turpis enim eget erat. Ut eros justo, ullamcorper ut sagittis eu, condimentum in arcu. Proin vel nisi ligula, vitae hendrerit purus. Vestibulum dignissim pulvinar consectetur. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Suspendisse ac eros risus.\
                           <br>\
                           Cras at massa quam. Cras eget dolor tortor. Mauris non nisl sapien. Maecenas dictum tincidunt erat nec tincidunt. Maecenas vestibulum lobortis varius. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Proin adipiscing tincidunt mauris quis fringilla. Maecenas sit amet ligula vitae neque tincidunt pulvinar quis id orci. Pellentesque sagittis tellus eget leo dapibus tempor. Duis eu diam non justo tincidunt gravida. Cras eros dolor, sodales eget elementum et, adipiscing in enim. Suspendisse at elit elit, id dignissim nisl. Donec imperdiet, quam vitae mattis hendrerit, leo arcu sagittis massa, a placerat urna sapien viverra tortor. Integer auctor, felis non condimentum ultrices, lectus ligula vestibulum eros, non pharetra libero purus eu purus. Duis ornare lacinia erat vitae sagittis. Sed fringilla volutpat magna, id auctor nisi elementum ut. Nulla facilisi. Quisque ut quam elit, ac consequat ligula.\
                           <br>\
                           Etiam fringilla lobortis mauris, in ullamcorper elit vestibulum eu. Ut dolor nisi, egestas vel consequat sit amet, faucibus vel nulla. Etiam vitae nibh ut dui imperdiet accumsan vel quis arcu. Suspendisse potenti. Ut vitae tellus risus, eu imperdiet eros. Nam vitae lacus a leo pulvinar feugiat. Donec augue mauris, suscipit porta euismod at, viverra vel magna. Cras luctus viverra neque, sit amet venenatis mi consequat id. Nulla consequat consequat nunc, eget mattis nisl tempor id. Praesent mattis sagittis tortor sed feugiat. Nunc vitae nisl est, a feugiat orci. Fusce in ipsum mauris. Etiam mattis sollicitudin nisi eu imperdiet. Duis in laoreet libero. Aenean eleifend est sit amet purus ultricies pretium. Morbi id nisl ut purus tempus sollicitudin quis non tellus. Donec sed quam ac nisl cursus euismod eu euismod eros.\
                           <br>\
                           Suspendisse mattis elementum sapien. Aliquam erat volutpat. Sed a tortor vitae nisi ornare vehicula. Praesent id faucibus sem. Mauris luctus ornare ipsum, id euismod lorem fermentum id. Maecenas egestas, mi sit amet porta facilisis, erat elit pretium nunc, ut mollis ipsum metus iaculis ligula. Duis semper ligula rhoncus nibh vehicula in scelerisque diam rhoncus. Vivamus quam magna, ultricies et posuere sed, mollis tristique est. Vivamus consectetur cursus diam, vitae pharetra nisi aliquam et. Nulla tortor sem, molestie quis pellentesque nec, tincidunt a libero. Proin tincidunt purus ut purus ornare dictum. Proin vehicula orci eu eros aliquam eu posuere turpis lobortis. Vivamus dapibus bibendum tristique. Donec facilisis fringilla mauris nec consequat. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Suspendisse ac nunc nec lacus interdum adipiscing eget vel quam. Duis rhoncus urna quis nisl elementum laoreet. Integer euismod tincidunt pulvinar.\
                           <br>\
                           Sed congue scelerisque blandit. Nam pharetra, nisi at auctor elementum, dolor elit bibendum mauris, congue feugiat arcu turpis non tortor. Cras tincidunt tellus sit amet nulla fermentum adipiscing. Mauris imperdiet purus vel nulla tempor vel elementum metus tempor. Aliquam eu dolor ut risus scelerisque commodo. Ut adipiscing nisl eget nisl mollis semper. Vestibulum arcu augue, sodales convallis venenatis ac, egestas euismod metus. Cras luctus elit sed nisl faucibus at accumsan lectus pharetra. Nunc rhoncus turpis est, vitae luctus turpis. Mauris tempus, velit sit amet feugiat consequat, tortor diam scelerisque urna, sit amet convallis est nisi quis dolor. Vivamus facilisis tellus quis sapien dapibus at sodales elit feugiat. Cras varius laoreet urna, eu tincidunt orci iaculis in. Sed quis lacus ac ligula aliquam egestas. Nam mattis massa sed magna lacinia pharetra. Sed semper faucibus mauris, ut consectetur libero tristique nec.\
                           <br>\
                           Proin vel elit id ante lacinia molestie quis in lectus. Nunc in ante nulla. Nam nec sem sed felis placerat rutrum. Sed pharetra molestie metus facilisis ornare. In sed nunc vel tellus volutpat dictum. Nunc risus enim, congue vel fermentum sed, varius eget elit. Proin vitae mi tortor. Pellentesque ut justo ligula. Nunc nibh elit, mattis at iaculis ac, pretium fermentum dui. Praesent in quam turpis. Quisque id velit eros. Nulla facilisi. Maecenas commodo dignissim libero nec pharetra. Praesent eget augue nec odio scelerisque sagittis. Donec nec dui dolor. Donec est nisi, faucibus vitae ultricies quis, cursus vel lorem. Nulla suscipit ultricies dui, vitae volutpat risus fermentum sit amet.\
                           <br>\
                           Morbi eu libero urna. Aliquam et nunc nec risus varius aliquet. Aenean semper sem sed purus ornare aliquam. Maecenas lobortis tristique nibh, id fermentum est egestas vel. Nam ut tortor pharetra augue rhoncus vulputate et et nisl. Donec scelerisque aliquet convallis. Quisque felis tellus, aliquet eu fermentum sed, mollis et lorem. Vivamus imperdiet, mauris at imperdiet tempor, elit justo aliquet purus, vitae vulputate dui elit euismod nulla. Nam nisl sem, laoreet at fermentum eu, eleifend vel quam. Duis venenatis hendrerit felis eget viverra. Aenean tincidunt ultrices pulvinar. Duis at risus tellus, sed euismod ipsum. Donec sit amet dui quam. Praesent at nunc a turpis sollicitudin faucibus. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Aliquam tincidunt accumsan lacinia. Nunc blandit gravida urna, quis venenatis metus mollis vel. Cras a nunc ac tortor aliquam fermentum ac sed purus. Etiam porttitor tincidunt risus, et scelerisque velit egestas vitae. Nulla at arcu sed sapien vehicula laoreet.\
                           <br>\
                           Pellentesque rhoncus odio id ante porttitor mollis. Nullam non mi vel erat suscipit lobortis. Vivamus urna neque, gravida ac interdum eget, commodo sit amet nunc. Maecenas libero nisl, bibendum eget tristique eget, laoreet vitae arcu. Maecenas porta diam vel libero tempus fermentum. Aenean molestie fringilla magna, tincidunt condimentum nunc aliquam nec. Mauris congue commodo lacus, in condimentum orci posuere id. Maecenas in ultrices ipsum. Nam sit amet orci nisi, ac lacinia nibh. Nullam molestie lorem eget felis interdum nec semper neque ultrices. Nunc et leo eu leo faucibus euismod. Suspendisse potenti. Vivamus id enim sed augue rutrum volutpat.\
                           <br>\
                           Curabitur facilisis placerat mi nec consectetur. Sed sed ante orci. Aliquam dapibus egestas dapibus. Aliquam feugiat urna ut libero adipiscing sed consectetur nulla mollis. Donec lacinia condimentum libero a egestas. Vivamus vestibulum ipsum ante, vitae fermentum libero. In hac habitasse platea dictumst. Ut sed quam libero. Aliquam bibendum sollicitudin faucibus. Nam elit felis, pulvinar vitae convallis id, fermentum sed leo. Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas. Donec dolor neque, vehicula commodo vulputate et, faucibus id nisi.\
                           <br>\
                           Vestibulum id metus in lectus iaculis aliquet rhoncus sit amet justo. Phasellus ac arcu ac felis auctor volutpat vitae nec mauris. Integer convallis, eros vel molestie tempor, odio enim posuere libero, quis iaculis elit ipsum eu eros. Sed metus diam, elementum ut scelerisque eu, scelerisque suscipit tellus. Aliquam congue fringilla sapien, vitae viverra arcu egestas sed. Donec quis lacus ipsum, ac faucibus eros. Suspendisse potenti. Cras non ipsum et dui tristique interdum. Vivamus ultrices vestibulum augue. Donec ac lacus quam, nec commodo eros. Quisque ac lacus lorem, vitae pellentesque nisl. Aliquam nec justo augue, quis porta tortor. Cras sit amet egestas eros. Duis neque ante, vulputate vel faucibus eu, sodales nec justo. Sed sagittis egestas orci sed lobortis. Donec eget consectetur nulla.\
                           <br>\
                           Vivamus mollis sodales ligula non imperdiet. Duis eget volutpat velit. Nunc eu dolor felis. Vestibulum massa augue, convallis in facilisis eget, vestibulum in mi. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Duis non ante enim, non volutpat massa. Mauris nunc turpis, dignissim non egestas vel, tristique id sem. Phasellus magna purus, scelerisque eget varius in, convallis id enim. Sed eu dolor neque, nec porta odio. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Sed aliquam pharetra diam id pharetra. Nulla lectus orci, condimentum sed malesuada vulputate, dictum quis purus.\
                           <br>\
                           Pellentesque quis lorem orci, commodo faucibus lectus. Nullam cursus, purus in porttitor tincidunt, enim ligula sagittis urna, eu fermentum velit eros a sapien. Maecenas ornare, mi eget faucibus malesuada, leo justo lobortis erat, et lobortis massa dolor ac augue. Nulla pulvinar malesuada lorem, id venenatis nulla pulvinar non. Praesent ac augue tortor, a feugiat risus. Vestibulum molestie ultrices felis vel fermentum. Maecenas feugiat eros non libero tempus condimentum sed sed metus. Sed sagittis metus et dui facilisis vitae vehicula massa convallis. Morbi sed ante nulla, nec posuere lorem. Donec euismod fringilla diam, in suscipit libero eleifend nec. Donec tristique scelerisque nibh sed euismod. Ut sed felis erat, ullamcorper vehicula sapien. Nunc pharetra interdum pretium. Nunc a mauris orci. Cras eu quam justo.\
                           <br>\
                           Proin ultricies dolor sit amet eros sollicitudin nec pulvinar tortor luctus. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Proin imperdiet turpis sit amet libero placerat in mattis quam molestie. Quisque luctus sem libero, vel venenatis purus. Ut vehicula egestas risus in pulvinar. Etiam ut vehicula est. Quisque commodo erat id est mattis pretium. Ut lobortis mauris a magna eleifend ultrices. Donec lacinia pharetra nisi, nec laoreet lacus varius et. Morbi risus diam, pulvinar a convallis id, blandit in lorem. Mauris quis tempor mi. Integer at nisi sapien. Integer dignissim consectetur tellus id rutrum. Nulla odio augue, sagittis in vestibulum ac, viverra convallis purus. Aliquam commodo mi id nibh bibendum vel viverra arcu consequat. Proin ipsum velit, ultrices id egestas nec, gravida eu dolor. Duis facilisis orci eu turpis feugiat ullamcorper. Integer venenatis metus non erat facilisis vel consectetur lectus varius. Nunc ac leo magna, a aliquet felis. Curabitur malesuada, arcu quis interdum lacinia, sapien sem condimentum ante, ac lacinia purus metus at orci.\
                           <br>\
                           Sed venenatis neque eu arcu bibendum imperdiet. Maecenas eget augue quis enim pretium cursus vitae venenatis dolor. Maecenas sit amet mauris mi. Aliquam luctus, dolor varius rhoncus rhoncus, diam turpis bibendum dui, eu hendrerit arcu nibh nec orci. Praesent condimentum, leo et commodo vulputate, purus odio interdum est, sed aliquam tellus lectus ut justo. Aenean pulvinar luctus dapibus. Nulla tempus, ante vel dictum ultricies, massa urna tincidunt libero, ut rhoncus risus turpis eget arcu. Morbi auctor commodo libero, in pretium quam vestibulum nec. Aenean ac tincidunt nulla. Cras vel orci enim. Praesent non metus ipsum.\
                           <br>\
                           Praesent ligula nulla, aliquam in tincidunt eu, lacinia eget neque. Integer tincidunt neque in risus malesuada gravida. Nullam ornare viverra nisl vel interdum. Nullam eget lectus a ipsum condimentum aliquam sit amet eu felis. Nam eu nisl enim. In pulvinar tincidunt ultrices. Morbi mi ipsum, placerat sit amet mollis vel, tincidunt vel nisi. In sed eros non magna sodales convallis pulvinar non turpis. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Fusce volutpat egestas elit, in lobortis urna egestas vitae. Nulla non tincidunt massa. Praesent ut convallis nunc. Morbi quis massa non lacus lobortis tempor non id libero. Proin turpis ante, porta et ullamcorper sit amet, malesuada vitae nisl. Suspendisse faucibus, ipsum quis varius porta, tellus augue porta arcu, nec posuere massa libero sed enim. Sed id suscipit ante. Ut fringilla sagittis orci porta tincidunt. Proin non massa sem. Nullam sollicitudin mauris et lorem euismod ut eleifend neque tempus.\
                           <br>\
                           Duis est est, rhoncus vel feugiat sed, iaculis id arcu. Donec ultrices lacinia dapibus. Fusce accumsan malesuada dolor, vitae viverra quam condimentum et. Sed id tellus lacus, ut sollicitudin turpis. Donec egestas ante et nunc consequat dapibus. Duis rutrum scelerisque orci, sollicitudin congue lectus condimentum non. In quis lacus et justo pellentesque vehicula. Nulla accumsan hendrerit lectus, non ornare nibh vehicula vitae. Sed iaculis dapibus laoreet. Nunc rutrum eleifend mi, ac interdum augue egestas nec. Morbi feugiat tincidunt eros, sit amet congue nisl ultricies sed. Nulla id est non dolor mattis lacinia. Mauris quis faucibus libero. Phasellus facilisis, massa a dapibus elementum, augue magna auctor tellus, nec placerat lorem lacus in purus. Morbi congue ipsum in dui euismod eleifend auctor eros eleifend. Pellentesque porttitor tincidunt pharetra. Maecenas quis tellus sed arcu convallis ultrices mollis ut neque.\
                           <br>\
                           Duis non sem ut mi facilisis pretium sit amet a erat. Suspendisse dignissim nisl nec risus tincidunt vitae ultrices arcu bibendum. Donec condimentum ullamcorper ipsum at vehicula. Donec interdum, odio vel blandit aliquet, nisi urna iaculis ligula, a ullamcorper mi ligula nec ligula. Curabitur pellentesque, augue id volutpat aliquet, lorem enim auctor sem, at tempus nibh augue ac diam. Etiam interdum lorem non nisi tempus vestibulum ut convallis elit. Cras cursus facilisis velit eget consectetur. Duis faucibus nulla sed leo accumsan et pharetra turpis aliquet. Curabitur ullamcorper tincidunt sapien lacinia suscipit. Curabitur consectetur, urna nec tempor lobortis, massa lectus accumsan turpis, vitae semper erat leo at dui. Sed ut facilisis velit. Ut vestibulum dapibus gravida. Quisque pellentesque auctor tortor eu malesuada. Vestibulum mauris dolor, tristique sit amet varius sit amet, varius eget neque. Curabitur in urna sit amet mi cursus pharetra sit amet at odio. Vivamus enim neque, tincidunt id adipiscing ac, volutpat non massa. Quisque gravida malesuada felis, eu aliquam nulla vestibulum id. Nullam eleifend ultricies cursus.\
                           <br>\
                           Pellentesque rutrum elementum orci, nec feugiat nisl vehicula nec. Suspendisse quis turpis ipsum. Maecenas eleifend nibh sit amet nisi auctor at auctor libero auctor. Suspendisse potenti. Morbi nec orci quis quam porttitor malesuada. Curabitur blandit luctus lacinia. Cras id turpis neque, a luctus justo. Morbi nec turpis risus. Fusce posuere urna sed nisl scelerisque mattis. Cras leo neque, dignissim ut vestibulum at, viverra in magna. Proin tellus leo, cursus eu porta id, eleifend in purus. Integer ut sapien leo, sed aliquet diam. Proin hendrerit imperdiet tincidunt.\
                           <br>\
                           Quisque rutrum urna non mi commodo vitae vestibulum velit tincidunt. Pellentesque vitae dolor et leo porta consectetur. Fusce semper eleifend tincidunt. In hac habitasse platea dictumst. Nulla mollis, dui in malesuada feugiat, sapien sapien porttitor nulla, at congue metus turpis vel arcu. Phasellus quis enim eu quam varius interdum quis lobortis magna. Quisque augue orci, suscipit eu malesuada ac, convallis id tortor. Fusce et tortor a mi fringilla congue. Praesent iaculis justo vel leo fringilla sodales. Nullam egestas, orci scelerisque placerat sagittis, diam tellus semper libero, in imperdiet neque sapien eget magna. Etiam eget massa orci, at interdum neque. Integer molestie enim eu sapien pellentesque lobortis a ac urna. Donec eu elit sit amet nunc bibendum faucibus et vel orci. Sed fringilla tristique sollicitudin. Donec rutrum pellentesque velit luctus suscipit. Aliquam lobortis sollicitudin ante at consectetur. Praesent nulla sapien, condimentum quis ornare quis, consequat sit amet arcu. Suspendisse potenti.\
                           <br>\
                           Ut volutpat lorem in lorem malesuada ultricies. Nunc nec eros lorem, ac iaculis velit. Donec vel metus lorem, a suscipit odio. Aliquam tincidunt nulla sit amet urna porttitor pulvinar. Cras adipiscing venenatis nibh a viverra. Etiam a metus nibh. Praesent sodales feugiat dolor, in scelerisque purus dapibus commodo. Mauris non mi metus. Fusce a aliquet ante. Aliquam erat volutpat. Praesent nec condimentum elit. Sed posuere orci et tortor volutpat sollicitudin. Vestibulum bibendum convallis dolor, non hendrerit velit consectetur eu. Aenean pharetra neque ac felis sollicitudin a ornare nunc accumsan. Aenean luctus dolor non purus lobortis sodales eget eget libero. Vivamus risus quam, vestibulum in ullamcorper et, vestibulum sit amet nunc. Ut odio felis, condimentum eget venenatis ut, suscipit ac sem. Nulla sit amet leo a metus hendrerit vehicula at vitae ante.\
                           <br>\
                           Aliquam diam leo, ultrices a sodales sed, laoreet id purus. Nullam eleifend leo vitae nisi euismod ac posuere elit tempus. Phasellus sed tellus est, eget fringilla diam. Aenean tempor enim quis nisi congue vitae placerat turpis vehicula. Proin congue, risus in pretium euismod, lacus tellus porttitor nulla, eget egestas lacus orci id orci. Proin nulla mauris, feugiat imperdiet consectetur id, condimentum ac libero. Donec consectetur leo a elit tincidunt vitae luctus nisl porta. Lorem ipsum dolor sit amet, consectetur adipiscing elit. Praesent sit amet orci quis dolor molestie commodo. Integer placerat risus neque. Phasellus non laoreet tellus. Nam pulvinar pellentesque nibh, egestas suscipit nisl gravida ac. Quisque sodales lorem imperdiet orci consequat ultricies. Phasellus fringilla diam id neque facilisis pharetra.\
                           <br>\
                           Nam scelerisque nulla ut neque tempor blandit volutpat nisl vehicula. Nam orci enim, luctus ut venenatis et, egestas id nunc. Phasellus risus sem, scelerisque sed vestibulum a, aliquam at sem. Sed venenatis tristique nisi id eleifend. Morbi urna tellus, cursus ac porttitor rhoncus, commodo at mi. Mauris ac magna nisi. Maecenas rutrum lobortis sapien, sed dapibus neque hendrerit a. Aliquam fringilla lectus nec arcu tempor et molestie dui suscipit. Proin et felis velit, vel pellentesque ligula. Donec tempus, mi at mollis porttitor, mauris urna congue elit, at varius nisi elit et est. Quisque eu libero lorem, eget tincidunt velit. Nullam hendrerit metus quis urna adipiscing quis tristique tortor condimentum. Integer iaculis scelerisque sem, vitae consectetur lorem facilisis eu. Phasellus consectetur blandit auctor. Morbi eleifend mollis ipsum, id facilisis leo euismod ut. Nunc id est tellus.";

typedef enum _slider_type{
    MOMENTUM_ANIM_DUR_MIN,
    MOMENTUM_ANIM_DUR_MAX,
    MOMENTUM_DISTANCE_MAX,
    MOMENTUM_THRESHOLD,
    MOMENTUM_SENSITIVITY_FRICTION,
    MOMENTUM_FRICTION,
    SLIDER_TYPE_MAX,
} slider_type;

typedef struct _slider_data{
    char name[256];
    int min;
    int max;
} slider_data;

slider_data sdata[] = {
    {"momentum anim dur min", 0.1, 2},
    {"momentum anim dur max", 0.1, 2},
    {"momentum distance max", 500, 5000},
    {"momentum threshold", 0, 100},
    {"momentum sensitivity friction", 0, 1},
    {"momentum friction", 0, 1},
};

static void
_changed_cb(void *data EINA_UNUSED, Evas_Object *obj, void *ev)
{
   double val = elm_slider_value_get(obj);
   printf("Changed to %1.2f for slider(%d)\n", val, (int)data);
   if(data == MOMENTUM_ANIM_DUR_MAX)
     {
        double min = elm_config_scroll_thumbscroll_momentum_animation_duration_min_limit_get();
        printf("val %f, min %f\n", val, min);
        if (val < min)
          {
             elm_slider_value_set(obj, min);
             elm_config_scroll_thumbscroll_momentum_animation_duration_max_limit_set(min);
          }
         else
          {
             elm_config_scroll_thumbscroll_momentum_animation_duration_max_limit_set(val);
          }
     }
   else if (data == MOMENTUM_ANIM_DUR_MIN)
     {
        double max = elm_config_scroll_thumbscroll_momentum_animation_duration_max_limit_get();
        if (val > max)
          {
             elm_slider_value_set(obj, max);
             elm_config_scroll_thumbscroll_momentum_animation_duration_min_limit_set(max);
          }
        else
          {
             elm_config_scroll_thumbscroll_momentum_animation_duration_min_limit_set(val);
          }
     }
   else if (data == MOMENTUM_DISTANCE_MAX)
     {
        elm_config_scroll_thumbscroll_momentum_distance_max_set(val);
     }
   else if (data == MOMENTUM_THRESHOLD)
     {
        elm_config_scroll_thumbscroll_threshold_set(val);
     }
   else if (data == MOMENTUM_SENSITIVITY_FRICTION)
     {
        elm_config_scroll_thumbscroll_sensitivity_friction_set(val);
     }
   else if (data == MOMENTUM_FRICTION)
     {
        elm_config_scroll_thumbscroll_momentum_friction_set(val);
     }
}

EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win = NULL;
   Evas_Object *scroller = NULL, *label = NULL, *rect = NULL;
   Evas_Object *box = NULL, *box2 = NULL, *tmp_box = NULL;
   Evas_Object *slider[SLIDER_TYPE_MAX] = {NULL,};

   int i = 0, j = 0;

   const static int TILE_SIZE = 50;
   const static int TILE_NUM = 50;
   //
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   elm_config_scroll_thumbscroll_momentum_animation_duration_min_limit_set(0.25);
   elm_config_scroll_thumbscroll_momentum_animation_duration_max_limit_set(1.25);

   //
   win = elm_win_util_standard_add("scroller", "Scroller");
   elm_win_autodel_set(win, EINA_TRUE);

   //
   box = elm_box_add(win);
   elm_box_horizontal_set(box, EINA_FALSE);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(box);
   elm_win_resize_object_add(win, box);

   //
   for (i = 0 ; i < SLIDER_TYPE_MAX; i ++)
     {
        slider[i] = elm_slider_add(box);
        evas_object_size_hint_align_set(slider[i], EVAS_HINT_FILL, 0.5);
        elm_object_text_set(slider[i], sdata[i].name);
        elm_slider_unit_format_set(slider[i], "%1.2f units");
        elm_slider_min_max_set(slider[i], sdata[i].min, sdata[i].max);
        evas_object_smart_callback_add(slider[i], "changed", _changed_cb, i);

        evas_object_show(slider[i]);
        elm_box_pack_end(box, slider[i]);
     }

   elm_slider_value_set(slider[MOMENTUM_ANIM_DUR_MIN], elm_config_scroll_thumbscroll_momentum_animation_duration_min_limit_get());
   elm_slider_value_set(slider[MOMENTUM_ANIM_DUR_MAX], elm_config_scroll_thumbscroll_momentum_animation_duration_max_limit_get());
   elm_slider_value_set(slider[MOMENTUM_DISTANCE_MAX], elm_config_scroll_thumbscroll_momentum_distance_max_get());

   elm_slider_value_set(slider[MOMENTUM_THRESHOLD], elm_config_scroll_thumbscroll_threshold_get());
   elm_slider_value_set(slider[MOMENTUM_SENSITIVITY_FRICTION], elm_config_scroll_thumbscroll_sensitivity_friction_get());
   elm_slider_value_set(slider[MOMENTUM_FRICTION], elm_config_scroll_thumbscroll_momentum_friction_get());

   //
   scroller = elm_scroller_add(box);
   evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
   elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_AUTO);
   elm_scroller_propagate_events_set(scroller, EINA_TRUE);

   box2 = elm_box_add(scroller);
   elm_box_horizontal_set(box2, EINA_FALSE);
   evas_object_size_hint_weight_set(box2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(box2);

   for (i = 0; i < TILE_NUM; i++)
     {
        tmp_box = elm_box_add(box2);
        elm_box_horizontal_set(tmp_box, EINA_TRUE);
        evas_object_size_hint_weight_set(tmp_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(tmp_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
        evas_object_show(tmp_box);
        for (j = 0; j < TILE_NUM; j++)
          {
             rect = evas_object_rectangle_add(evas_object_evas_get(tmp_box));
             evas_object_size_hint_min_set(rect, TILE_SIZE, TILE_SIZE);
             evas_object_color_set(rect, rand()%128+128, rand()%128+128, rand()%128+126, 255);
             evas_object_show(rect);
             elm_box_pack_end(tmp_box, rect);

          }
        elm_box_pack_end(box2, tmp_box);
        evas_object_show(tmp_box);
     }
   elm_object_content_set(scroller, box2);
   evas_object_show(scroller);
   elm_box_pack_end(box, scroller);

   //
   evas_object_resize(win, 400, 800);
   evas_object_show(win);

   elm_run();


   return 0;
}
ELM_MAIN()
