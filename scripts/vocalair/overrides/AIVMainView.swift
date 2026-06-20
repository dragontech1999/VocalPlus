import SwiftUI
import CoreAudioKit
import AIVFramework

public struct AIVMainView: View {
    @StateObject public var viewModel = AudioUnitViewModel()
    public var audioUnit: AIVDemo?

    private let rackScale: CGFloat = 1.25

    public init(audioUnit: AIVDemo? = nil) {
        self.audioUnit = audioUnit
    }
    
    public var body: some View {
        ZStack {
            Color(red: 0.08, green: 0.08, blue: 0.10).edgesIgnoringSafeArea(.all)
            
            VStack(spacing: 0) {
                HStack {
                    Text("VocalAir+")
                        .font(.system(size: 20, weight: .bold, design: .rounded))
                        .foregroundColor(.white)
                        .tracking(1.5)
                        .shadow(color: .white.opacity(0.2), radius: 5)
                    
                    Spacer()
                    
                    Text("RACK · 125%")
                        .font(.caption)
                        .foregroundColor(.gray)
                        .padding(.horizontal, 8)
                        .padding(.vertical, 4)
                        .background(Color.white.opacity(0.1))
                        .cornerRadius(4)
                }
                .padding()
                .background(Color(red: 0.12, green: 0.12, blue: 0.14))
                .shadow(radius: 5)
                .zIndex(1)
                
                GeometryReader { geometry in
                    ScrollView(.vertical, showsIndicators: true) {
                        VStack(spacing: 15) {
                            OutputPanel(viewModel: viewModel)
                            InputPanel(viewModel: viewModel)
                            GatePanel(viewModel: viewModel)
                            PitchDeesserPanel(viewModel: viewModel)
                            EQPanel(viewModel: viewModel)
                            DynamicsPanel(viewModel: viewModel)
                            SpatialPanel(viewModel: viewModel)
                        }
                        .padding()
                        .padding(.bottom, 40)
                        .scaleEffect(rackScale, anchor: .top)
                        .frame(width: geometry.size.width / rackScale, alignment: .top)
                    }
                }
            }
        }
        .onAppear {
            if let au = audioUnit {
                viewModel.connect(audioUnit: au)
            }
        }
        .onChange(of: audioUnit) { newAU in
             if let au = newAU {
                 viewModel.connect(audioUnit: au)
             }
        }
    }
}
