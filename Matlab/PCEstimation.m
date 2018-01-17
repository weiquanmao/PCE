function [AccStatic, FScore] = PCEstimation(FilePath_ER2S, FilePath_ES2R, FilePath_Box, FilePath_OutFig)
%%
%
%  AccStatic := [AMiu, ASTD, A50, A90]
%  FScore    := [F002, A002, C002; ...
%                F005, A005, C005]
%
%%
    k_    = 0.01;   
    maxk_ = 0.1;
    N_    = 50;
    
    E_Acc = importdata(FilePath_ER2S);
    E_Com = importdata(FilePath_ES2R);
    Box   = importdata(FilePath_Box);
    DU    = Box(1,3) * k_;
    MU    = Box(1,3) * maxk_;
    U002  = DU*2;
    U005  = DU*5;
    
    [pathstr, name, ~] = fileparts(FilePath_OutFig);
    FigFile = [pathstr '/' name '.fig'];
    PngFile = [pathstr '/' name '.png'];
    SpLit = regexp(name, '_', 'split');
    TarName = upper(SpLit{1});
    %% For Accuary
    E_Acc_Sel = E_Acc(E_Acc<MU);  
    E_Acc_Sel = sort(E_Acc_Sel);
    
    N_AllR = length(E_Acc);
    N_SelR = length(E_Acc_Sel);
    
    AMiu = mean(E_Acc_Sel);
    ASTD = std(E_Acc_Sel);
    A50  = median(E_Acc_Sel);
    A90  = E_Acc_Sel(round(N_SelR*0.9+0.5));
    A002 = length(find(E_Acc_Sel<U002))/N_SelR * 100;
    A005 = length(find(E_Acc_Sel<U005))/N_SelR * 100;
    
    E_Acc_Sel = E_Acc_Sel/DU;
    %% For Completeness
    N_AllS = length(E_Com);
    
    C002 = length(find(E_Com<U002))/N_AllS * 100;
    C005 = length(find(E_Com<U005))/N_AllS * 100;
    %% For F-Score
    F002 = 2*(A002*C002)/(A002+C002);
    F005 = 2*(A005*C005)/(A005+C005);
    
    %% Rresult
    AccStatic = [AMiu, ASTD, A50, A90];
    FScore    = [F002, A002, C002; ...
                 F005, A005, C005];
             
    %% To Draw     
    figure();
    
    subplot(2,1,1);
    hist(E_Acc_Sel,N_);
    HMax = max(hist(E_Acc_Sel,N_));
    WMax = maxk_/k_;
    hold on; 
    plot([AMiu/DU,AMiu/DU],[0,HMax*1.2],'r'); 
    plot([A50/DU,A50/DU],[0,HMax*1.2],'c');
    plot([A90/DU,A90/DU],[0,HMax*1.2],'g');
    hold off;
    ylim([0 HMax*1.2]);
    xlim([0 WMax]);
    title([TarName ' ( #NPts_{GT} : ' num2str(N_AllR) ', #NPts_{Ref} : ' num2str(N_AllS) ' )']);
    legend(['Total:' num2str(N_SelR) '(+' num2str(N_AllR-N_SelR) ')'],'Mean value','Acc50','Acc90');
    ylabel('NPts');
    text(max(E_Acc_Sel)*0.8, -HMax*0.16,['1DU=0.01L_D=' num2str(DU,'%.4f')],'FontWeight','bold');  

    PH = subplot(2,1,2);
    H = 5;
    LE = AMiu/DU * 10;
    LD = ASTD/DU * 10;
    L50 = A50/DU * 10;
    L90 = A90/DU * 10;
    hold on
    patch([0,L90,L90,0],[0,0,H,H],'g');
    if (L50>LE)
        patch([0,L50,L50,0],[0,0,H,H],'c');
    end
    patch([0,LE,LE,0],[0,0,H,H],'w');
    if LE>LD
        plot([LE-LD, LE-LD],[H/2-2, H/2+2] ,'r');
        plot([LE-LD, LE],[H/2, H/2] ,'r');  
    else
        plot([0, LE],[H/2, H/2] ,'r'); 
    end
    plot(LE,H/2,'ro');
    plot([LE, LE+LD],[H/2, H/2] ,'r');
    plot([LE+LD, LE+LD],[H/2-2, H/2+2] ,'r');
    if (L50<LE)
        patch([0,L50,L50,0],[0,0,H,H],'c');
    end
    HH1 = -2.5;
    HH2 = -5.5;
    text(0, HH1, ...
        [...
        [num2str(AMiu,'%.3f') num2str(AMiu/DU,'(%.2fDU)') '¡À'], ...
        [num2str(ASTD,'%.3f') num2str(ASTD/DU,'(%.2fDU)')]...
        ],...
        'fontsize',12,'FontWeight','bold');

    text(42,HH1,['Acc50:' num2str(A50,'%.3f(') num2str(A50/DU,'%.2fDU)')], ...
        'Color','c','fontsize',12,'FontWeight','bold');
    text(74,HH1,['Acc90:' num2str(A90,'%.3f(') num2str(A90/DU,'%.2fDU)')], ...
        'Color','g','fontsize',12,'FontWeight','bold');
    
    text(0, HH2, ...
        {[
         ['[ ' num2str(A002,'A002:%.1f') '%' ', ' num2str(C002,'C002:%.1f') '%' ', ' 'F002:' num2str(F002,'%.1f') '%' ' ]'], ...
         [' [ ' num2str(A005,'A005:%.1f') '%' ', ' num2str(C005,'C005:%.1f') '%' ', ' 'F005:' num2str(F005,'%.1f') '%' ' ]']...
         ]}, ...
        'Color','m','fontsize',12,'FontWeight','bold');
   
    text(0, -15, {...
        'In >>', ...
        strrep(FilePath_ER2S,'_','\_'), ...
        strrep(FilePath_ES2R,'_','\_'), ...
        strrep(FilePath_Box,'_','\_'), ...
        strrep(FilePath_OutFig,'_','\_')});
    
    text(0, -30, {'Out >>',strrep(FigFile,'_','\_'),strrep(PngFile,'_','\_')});
    
    axis equal; axis off;
    xlim([0 WMax*10]);
    ylim([0 H]);

    PPP=get(PH,'pos');
    PPP(2)=PPP(2)+0.24;
    set(PH,'pos',PPP);

    hold off  
    set(gcf,'position',[200, 100, 800, 600]);
    set(gcf,'color','white')
    saveas(gcf,FigFile);
    Fig=getframe(gcf);
    imwrite(Fig.cdata,PngFile);
end